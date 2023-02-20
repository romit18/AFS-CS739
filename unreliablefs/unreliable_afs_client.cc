#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <libgen.h>
#include "unreliable_afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;
using grpc::ServerWriter;

using unreliable_afs::UnreliableAFSProto;
using unreliable_afs::MkdirRequest;
using unreliable_afs::MkdirReply;
using unreliable_afs::RmdirRequest;
using unreliable_afs::RmdirReply;
using unreliable_afs::GetAttrRequest;
using unreliable_afs::GetAttrReply;
// using unreliable_afs::GetXAttrRequest;
// using unreliable_afs::GetXAttrReply;
using unreliable_afs::OpenDirRequest;
using unreliable_afs::OpenDirReply;
using unreliable_afs::OpenRequest;
using unreliable_afs::OpenReply;
using unreliable_afs::CloseRequest;
using unreliable_afs::CloseReply;
using unreliable_afs::ReadDirRequest;
using unreliable_afs::ReadDirReply;

// Useful for create - mkdir if it doesn't exist
// Source: https://stackoverflow.com/a/9210960
int mkpath(char* file_path, mode_t mode) {
    assert(file_path && *file_path);
    for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';
        if (mkdir(file_path, mode) == -1) {
            if (errno != EEXIST) {
                *p = '/';
                return -1;
            }
        }
        *p = '/';
    }
    return 0;
}

extern "C"{
class UnreliableAFS {

    public:
    UnreliableAFS(std::shared_ptr<Channel> channel)
        : stub_(UnreliableAFSProto::NewStub(channel)) {}


    int Mkdir(const std::string& path, int mode) {
        MkdirRequest request;
        request.set_path(path);
        request.set_mode(mode);

        MkdirReply reply;
        ClientContext context;
        Status status = stub_->Mkdir(&context, request, &reply);

        return status.ok() ? reply.err() : -1;
    }

    int Rmdir(const std::string& path) {
        RmdirRequest request;
        request.set_path(path);

        RmdirReply reply;
        ClientContext context;
        Status status = stub_->Rmdir(&context, request, &reply);

        return status.ok() ? reply.err() : -1;
    }

    int GetAttr(const std::string& path, struct stat *stbuf){
        GetAttrRequest request;
        request.set_path(path);

        GetAttrReply reply;
        ClientContext context;
        Status status = stub_->GetAttr(&context, request, &reply);
        if (status.ok()) {
            memcpy(stbuf, (struct stat *)(reply.buf()).data(), sizeof(struct stat*));
            return reply.err();
        } else {
            return -1;
        }
    }

    long int GetXAttr(const std::string& path, const std::string& name, void* value, size_t size){
        GetXAttrRequest request;
        request.set_path(path);
        request.set_name(name);
        request.set_size(size);

        GetXAttrReply reply;
        ClientContext context;
        Status status = stub_->GetXAttr(&context, request, &reply);
        if (status.ok()) {
            if (reply.size() > 0) {
                memcpy(value, (struct stat *)(reply.value()).data(), reply.size());
	    }
            return reply.size();
        } else {
            return -1;
        }
    }

    int Opendir(const std::string& path, DIR* directory){
        OpenDirRequest request;
        request.set_path(path);

        OpenDirReply reply;
        ClientContext context;
        Status status = stub_->OpenDir(&context, request, &reply);
        if (status.ok()) {
            memcpy(directory, (DIR*)reply.dir().data(), sizeof(DIR*));
            directory = (DIR *)reply.dir().data();
            return reply.err();
        } else {
            return -1;
        }
    }

    int Readdir(const std::string& path, char** buf) {
        ReadDirRequest request;
        request.set_path(path);

        ReadDirReply reply;
        ClientContext context;
        std::unique_ptr<ClientReader<ReadDirReply>> reader(stub_->ReadDir(&context, request));
       // Status status = stub_->ReadDir(&context, request, &reply);
        int i=0;
        while (reader->Read(&reply)) {
            buf[i]=(char *)malloc(sizeof(reply.buf()));
            strcpy(buf[i], reply.buf().c_str());
          //  buf[i]=reply.buf().c_str();
            i=i+1;
            if (reply.err() < 0) {
                break;
            }
        }

        Status status = reader->Finish();

        return status.ok() ? reply.err() : -1;
    }

    int Open(const std::string& path, int flags){
    // Open logic:
    // Issue getattr. If the file doesn't exist on server,
    // but exists locally, open - otherwise throw error
    // If it exists on server, also run getattr locally
    // If it doesn't exist in cache, fetch
    // Compare the attributes. If modified time on server
    // is after modified time on client, fetch
    // Otherwise, local copy is up to date, return 0
    // If it doesn't exist, return error
        OpenRequest request;
        request.set_path(path);
        request.set_flags(flags);

        struct stat rpcbuf;

        int ret = GetAttr(path, &rpcbuf);
        if (ret < 0) {
	    // We delete the file if it is a stale copy
	    // If it was newly created we cannot access, since O_EXCL was used
            int rc = open(path.c_str(), flags);
            if ((rc == -1 ) && (errno == EEXIST)) {
                return -errno;
            } else {
		unlink(path.c_str());
	    }
            return rc;
        }

        OpenReply reply;
        ClientContext context;
        struct stat file_stats;
        int local_res = lstat(path.c_str(), &file_stats);
	// If modified time at server is after modified time at client,
	// or if file is not present in cache, fetch it to cache
	if (difftime(rpcbuf.st_mtime, file_stats.st_mtime) > 0){
	    char * tmp_path = (char *) malloc(path.size() + 8);
	    snprintf(tmp_path, path.size() + 7, "%s.tmpbak", path.c_str());
	    // Fetch from server
            Status status = stub_->Open(&context, request, &reply);
            if (status.ok()) {
		// Allocate space for fetched file and fetch
		char* fetched_file = (char *) malloc(reply.num_bytes());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
		// remove previous copy and write updated file
		// write new file to temporary copy, then unlink
		// old copy and rename temporary copy to new file
		// int new_file = open(tmp_path, flags | O_CREAT, 0644);
		int new_file = open(tmp_path, flags | O_CREAT, 0777);
		write(new_file, fetched_file, reply.num_bytes());
		fsync(new_file);
		// Reset file offset of open fd
		lseek(new_file, SEEK_SET, 0);
		// Close the file and rename it
		close(new_file);
		unlink(path.c_str());
		rename(tmp_path, path.c_str());
		// Return fd
		new_file = open(path.c_str(), flags);
		return new_file;
                // return reply.err();
            } else {
                return -1;
            }
	} else if ((local_res == -1) && (errno == ENOENT)){
	    // Fetch from server
            Status status = stub_->Open(&context, request, &reply);
            if (status.ok()) {
		// Allocate space for fetched file and fetch
		char* fetched_file = (char *) malloc(reply.num_bytes());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
		// Create directories in path (if not present) and write file
                mkpath(const_cast<char*>(path.c_str()), 0777);
		int new_file = open(path.c_str(), flags | O_CREAT, 0777);
		// int new_file = open(path.c_str(), flags | O_CREAT, 0644);
		write(new_file, fetched_file, reply.num_bytes());
		fsync(new_file);
		// Reset file offset of open fd
		lseek(new_file, SEEK_SET, 0);
		// Return fd
		return new_file;
                // return reply.err();
            } else {
                return -1;
            }
	} else if(difftime(rpcbuf.st_mtime, file_stats.st_mtime) <= 0) {
		return open(path.c_str(), flags);
	}

    }

    int Create(const std::string& path, int flags, int mode){
    // Create logic:
    // Issue getattr. If the file doesn't exist on server,
    // create it locally, and flush on close
    // If it exists on server, also run getattr locally
    // If it doesn't exist, fetch
    // Compare the attributes. We should not have to check for
    // modified time on server being after modified time on client
    // since create is only called for new files
        OpenRequest request;
        request.set_path(path);
        request.set_flags(flags);
        // request.set_mode(mode);

        struct stat rpcbuf;

	// Check if the directory in which this file
	// supposedly resides exists on server.
	// If it doesn't, return.
	char * file_dirname = (char *) malloc(PATH_MAX);
	file_dirname = dirname(const_cast<char*>(path.c_str()));
	int directory_exist = GetAttr(file_dirname, &rpcbuf);
	if (directory_exist < 0) {
		return -errno;
	}

        int ret = GetAttr(path, &rpcbuf);
        if (ret < 0){
            mkpath(const_cast<char*>(path.c_str()), 777);
            // mkpath(const_cast<char*>(path.c_str()), mode);
            int rc = open(path.c_str(), flags, mode);
            if (rc == -1) {
                return -errno;
            }
            return rc;
        }

        OpenReply reply;
        ClientContext context;
        struct stat file_stats;
        int local_res = lstat(path.c_str(), &file_stats);
	if ((local_res == -1) && (errno == ENOENT)) {
	    // Fetch from server
            Status status = stub_->Open(&context, request, &reply);
            if (status.ok()) {
		// Allocate space for fetched file and fetch
		char* fetched_file = (char *) malloc(reply.num_bytes());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
		// Create directories in path (if not present) and write file
                mkpath(const_cast<char*>(path.c_str()), 0777);
		int new_file = open(path.c_str(), flags | O_CREAT | O_EXCL, mode);
		write(new_file, fetched_file, reply.num_bytes());
		fsync(new_file);
		// Reset file offset of open fd
		lseek(new_file, SEEK_SET, 0);
		// Return fd
		return new_file;
                // return reply.err();
            } else {
                return -1;
            }
	}
    }

    int Close(const std::string& path, int fd){
        CloseRequest request;
        request.set_path(path);
	std::cout<<"Closing File:"<<path<<std::endl;
	int res;

	res = fsync(fd);
	if (res == -1) {
            return -errno;
	}

	struct stat file_info;
	res = fstat(fd, &file_info);
	if (res == -1) {
            return -errno;
	}

	off_t file_size = file_info.st_size;
	char * buf = (char *) malloc(file_size);
	pread(fd, buf, file_size, 0);
	close(fd);

        request.set_path(path);
	request.set_file(std::string(buf, file_size));
	request.set_num_bytes(file_size);

        CloseReply reply;
        ClientContext context;
        Status status = stub_->Close(&context, request, &reply);
        return status.ok() ? reply.err() : -1;
    }

    private:
    std::unique_ptr<UnreliableAFSProto::Stub> stub_;
    
};

UnreliableAFS* NewUnreliableAFS(){
  return new UnreliableAFS(grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials()));
}

int Mkdir(UnreliableAFS* unreliableAFS, const char* path, int mode){
  return unreliableAFS->Mkdir(path, mode);
}

int Rmdir(UnreliableAFS* unreliableAFS, const char* path){
  return unreliableAFS->Rmdir(path);
}

int Getattr(UnreliableAFS* unreliableAFS, const char* path, struct stat* stbuf){
  return unreliableAFS->GetAttr(path, stbuf);
}

long int Getxattr(UnreliableAFS* unreliableAFS, const char* path, const char* name, void* value, size_t size){
  return unreliableAFS->GetXAttr(path, name, value, size);
}

int Opendir(UnreliableAFS* unreliableAFS, const char* path, DIR* directory){
  return unreliableAFS->Opendir(path, directory);
}

int Readdir(UnreliableAFS* unreliableAFS, const char* path, char** buf){
    return unreliableAFS->Readdir(path, buf);
}

int Open(UnreliableAFS* unreliableAFS, const char* path, int flags){
  return unreliableAFS->Open(path, flags);
}

int Create(UnreliableAFS* unreliableAFS, const char* path, int flags, int mode){
  return unreliableAFS->Create(path, flags, mode);
}

int Close(UnreliableAFS* unreliableAFS, const char* path, int fd){
  return unreliableAFS->Close(path, fd);
}

// int main(int argc, char** argv) {
//   // Instantiate the client. It requires a channel, out of which the actual RPCs
//   // are created. This channel models a connection to an endpoint specified by
//   // the argument "--target=" which is the only expected argument.
//   // We indicate that the channel isn't authenticated (use of
//   // InsecureChannelCredentials()).
//   std::string target_str;
//   std::string arg_str("--target");
//   if (argc > 1) {
//     std::string arg_val = argv[1];
//     size_t start_pos = arg_val.find(arg_str);
//     if (start_pos != std::string::npos) {
//       start_pos += arg_str.size();
//       if (arg_val[start_pos] == '=') {
//         target_str = arg_val.substr(start_pos + 1);
//       } else {
//         std::cout << "The only correct argument syntax is --target="
//                   << std::endl;
//         return 0;
//       }
//     } else {
//       std::cout << "The only acceptable argument is --target=" << std::endl;
//       return 0;
//     }
//   } else {
//     target_str = "localhost:50051";
//   }
//   UnreliableAFS greeter(
//       grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
//   std::string dirpath("/world");
//   int res = greeter.Mkdir(dirpath, 0777);
//   std::cout << "Greeter received: " << res << std::endl;


//   return 0;
// }
}
