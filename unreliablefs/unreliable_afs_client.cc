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
#include <fuse.h>
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
using unreliable_afs::GetXAttrRequest;
using unreliable_afs::GetXAttrReply;
using unreliable_afs::OpenDirRequest;
using unreliable_afs::OpenDirReply;
using unreliable_afs::OpenRequest;
using unreliable_afs::OpenReply;
using unreliable_afs::OpenMRequest;
using unreliable_afs::OpenMReply;
using unreliable_afs::ReadMRequest;
using unreliable_afs::ReadMReply;
using unreliable_afs::WriteMRequest;
using unreliable_afs::WriteMReply;
using unreliable_afs::CloseRequest;
using unreliable_afs::CloseReply;
using unreliable_afs::UnlinkRequest;
using unreliable_afs::UnlinkReply;
using unreliable_afs::RenameRequest;
using unreliable_afs::RenameReply;
using unreliable_afs::ReadDirRequest;
using unreliable_afs::ReadDirReply;
using unreliable_afs::AccessRequest;
using unreliable_afs::AccessReply;

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
            memcpy(stbuf, (reply.buf()).data(), sizeof(struct stat));
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

    DIR* Opendir(const std::string& path){
        OpenDirRequest request;
        request.set_path(path);

        OpenDirReply reply;
        ClientContext context;
        Status status = stub_->OpenDir(&context, request, &reply);
        if (status.ok()) {
            DIR* directory = (DIR*) malloc(reply.dir().size());
            memcpy(directory, (DIR*)reply.dir().data(), reply.dir().size());
            // directory = (DIR *)reply.dir().data();
            return directory;
        } else {
            return 0;
        }
    }

    int Readdir(const std::string& path, void* buf, fuse_fill_dir_t filler) {
        ReadDirRequest request;
        request.set_path(path);

        ReadDirReply reply;
        ClientContext context;
        std::unique_ptr<ClientReader<ReadDirReply>> reader(stub_->ReadDir(&context, request));
       // Status status = stub_->ReadDir(&context, request, &reply);
        // int i=0;
        while (reader->Read(&reply)) {
            if(reply.err()==0) {
                filler(buf, (reply.name()).c_str(), NULL, 0);
            }
            // buf.push_back(reply.buf());
        //     const char* bufi=reply.buf().c_str();
        //     buf[i]=(char *)malloc(sizeof(bufi));

        //     // buf[i]=(char *)malloc(sizeof(reply.buf()));
        //     strcpy(buf[i], bufi);
        //   //  buf[i]=reply.buf().c_str();
        //     i=i+1;
            // if (reply.err() < 0) {
            //     break;
            // }
        }
        // std::cout << "Greeter received: " << buf.c_str() << std::endl; 
        Status status = reader->Finish();

        return status.ok() ? reply.err() : -1;
    }

    int Access(const std::string& path, int mode) {
        AccessRequest request;
        request.set_path(path);
        request.set_mode(mode);

        AccessReply reply;
        ClientContext context;
        Status status = stub_->Access(&context, request, &reply);

        return status.ok() ? reply.err() : -1;
    }


    int OpenM(const std::string& path, fuse_file_info* fi) {

        OpenMRequest request;
        OpenMReply reply;
        ClientContext context;
        request.set_path(path);
        request.set_flag(fi->flags);
    
        struct stat server_stat;
        int res = GetAttr(path, &server_stat);

        if (res < 0) { // Doesn't exist on server, create one on server & return local
            printf("Client.OpenM>> File doesn't exist on server, creating \n");
            Status status = stub_->OpenM(&context, request, &reply);
            if(status.ok()){
                if(reply.err() > 0) {
                    mkpath(const_cast<char*>(path.c_str()), 0777);
                    fi->fh = open(path.c_str(), fi->flags, 0777);
                    return fi->fh;
                }
                return -1;
            }
            return -1;
        }

        struct stat file_stats;
        int local_res = lstat(path.c_str(), &file_stats);
        printf("Client.OpenM>> local_res: %d , errno: %d\n", local_res, errno);
        printf("Client.OpenM>> server_stat.st_mtim.tv_sec:server_stat.st_mtim.tv_nsec \n\t\t %ld : %ld\n", server_stat.st_mtim.tv_sec, server_stat.st_mtim.tv_nsec);
        printf("Client.OpenM>> file_stats.st_mtim.tv_sec:file_stats.st_mtim.tv_nsec \n\t\t %ld : %ld\n", file_stats.st_mtim.tv_sec, file_stats.st_mtim.tv_nsec);
        if ((local_res == -1 && errno == ENOENT) || 
          (server_stat.st_mtim.tv_sec > file_stats.st_mtim.tv_sec) || ((server_stat.st_mtim.tv_sec == file_stats.st_mtim.tv_sec) 
          && (server_stat.st_mtim.tv_nsec > file_stats.st_mtim.tv_nsec))){ // Fetch from server

            std::string rpcbuf;
            int size = server_stat.st_size;
            printf("Client.OpenM>>  local file didn't exist / stale file >> fetching st size: %d\n", size);
            res = ReadM(path, rpcbuf, size, 0);
            if (res < 0) {
                return res;
            }
            
            mkpath(const_cast<char*>(path.c_str()), 0777); // save to local disk
            
            int fd = open(path.c_str(), O_RDWR | O_CREAT | O_EXCL, 0644);
            if ((fd == -1) && (EEXIST == errno)) {
                fd = open(path.c_str(), O_TRUNC | O_RDWR); // open with truncate
                if (fd == -1) {
                    return -errno;
                } 
            }
            fi->fh=fd;
            int res = pwrite(fd, &rpcbuf[0], rpcbuf.size()+1, 0 /*offset*/);
            if(res<0)
                return -1;
            // lseek(fd, (size_t)0, SEEK_CUR);
        }
        else {
            printf("Client.OpenM >>  updated local file exists: %s", path.c_str());
            fi->fh = open(path.c_str(), fi->flags);
        }
        return fi->fh;
    }

    int ReadM(const std::string& path, std::string& buf, int size, int offset) {
        ReadMRequest request;
        request.set_path(path);
        request.set_size(size);
        request.set_offset(offset);

        ReadMReply reply;
        ClientContext context;

        std::unique_ptr<ClientReader<ReadMReply> > reader(stub_->ReadM(&context, request));
        buf = std::string();
        buf.reserve(size);

        while (reader->Read(&reply)) {
            buf += reply.buf();
            if (reply.num_bytes() < 0) {
                break;
            }
        }
        Status status = reader->Finish();
        return status.ok() ? buf.size() : -1;
    }

    int WriteM(const std::string& path, std::string& data, int size, int offset) {
        WriteMRequest request;
        WriteMReply reply;
        std::cout<<"WriteM: "<<path<<std::flush;
        ClientContext context;
        std::unique_ptr<ClientWriter<WriteMRequest> > writer(
                stub_->WriteM(&context, &reply));
        
        int remain = size;
        int stump = 1048576; // 1Mb
        int curr = offset;
        std::cout<<"WriteM before while: "<<remain<<std::flush;
        while (remain >= 0) {
            std::cout<<"WriteM inside while: "<<remain<<std::flush;
            request.set_path(path); 
            request.set_buf(data.substr(curr, std::min(stump, remain)));
            request.set_size(std::min(stump, remain));
            request.set_offset(curr);
            curr += stump;
            remain -= stump;
            if (!writer->Write(request)) {
                // Broken stream.
                break;
            }
        }
        writer->WritesDone();
        Status status = writer->Finish();

        return status.ok() ? reply.num_bytes() : -1;
    }

    int CloseM(const std::string& path, fuse_file_info* fi){
        // std::cout<<"Client.CloseM >>  Flushing & local file exists: "<<path<<std::flush;
        if ((int)fi->fh != -1) {
            int rc = fsync(fi->fh);
	        if (rc == -1) {
                return -errno;
	        }
            std::string buf;
            lseek(fi->fh, 0, SEEK_SET);
            int size = lseek(fi->fh, (size_t)0, SEEK_END);
            std::cout<<"Client.CloseM: size:"<<size<<std::flush;
            if(size <= 0){
                return 0;
            }
            buf.resize(size);
            lseek(fi->fh, 0, SEEK_SET);
            fsync(fi->fh);
            int res = pread(fi->fh, &buf[0], size+1, 0);
            if(res < 0) {
                std::cout<<"Client.CloseM: res < 0"<<std::flush;
                return -1;
            }

            std::cout<<"Client.CloseM: buf contents: "<<buf<<std::flush;

            res = WriteM(path, buf, buf.size(), 0 /*offset*/);                                                                                      
            if (res < 0) {                                                                                                                                                  
                printf("Client.CloseM >> Failed to write to server");                                                                                                                                   
                return res;                                                                                                                                                 
            }
            return res;
        }
        return -1;
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
        // Dump stats into temporary file
        char * stats_file_path = (char *) malloc(path.size() + 16);
        snprintf(stats_file_path, path.size() + 15, "%s.file_stats_tmp", path.c_str());
        int stats_file_fd = open(stats_file_path, O_RDWR | O_CREAT, 0777);
        // If modified time at server is after modified time at client,
        // or if file is not present in cache, fetch it to cache
        if ((rpcbuf.st_mtim.tv_sec > file_stats.st_mtim.tv_sec) || ((rpcbuf.st_mtim.tv_sec == file_stats.st_mtim.tv_sec) && (rpcbuf.st_mtim.tv_nsec > file_stats.st_mtim.tv_nsec))){
            char * tmp_path = (char *) malloc(path.size() + 8);
            snprintf(tmp_path, path.size() + 7, "%s.tmpbak", path.c_str());
            // Fetch from server
            Status status = stub_->Open(&context, request, &reply);
            if (status.ok()) {
                // Allocate space for fetched file and fetch
		        char* fetched_file = (char *) malloc(reply.num_bytes());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
                // remove previous copy and write updated file
                // write new file to temporary copy, then unlink
                // old copy and rename temporary copy to new file
                // int new_file = open(tmp_path, flags | O_CREAT, 0644);
                int new_file = open(tmp_path, flags | O_CREAT, 0777);
                write(new_file, fetched_file, reply.num_bytes());
                fsync(new_file);
                fsync(new_file);
                // Reset file offset of open fd
                lseek(new_file, SEEK_SET, 0);
                // Close the file and rename it
                close(new_file);
                unlink(path.c_str());
                rename(tmp_path, path.c_str());
                // Copy stats into temporary file
                local_res = lstat(path.c_str(), &file_stats);
                pwrite(stats_file_fd, &file_stats, sizeof(struct stat), 0);
                fsync(stats_file_fd);
                close(stats_file_fd);
                // Return fd
                new_file = open(path.c_str(), flags);
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
                printf("characters: %s", reply.file().data());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
                memcpy(fetched_file, (char *)reply.file().data(), reply.num_bytes());
                // Create directories in path (if not present) and write file
                mkpath(const_cast<char*>(path.c_str()), 0777);
                int new_file = open(path.c_str(), flags | O_CREAT, 0777);
                // int new_file = open(path.c_str(), flags | O_CREAT, 0644);
                write(new_file, fetched_file, reply.num_bytes());
                fsync(new_file);
                fsync(new_file);
                // Reset file offset of open fd
                lseek(new_file, SEEK_SET, 0);
                // Copy stats into temporary file
                local_res = lstat(path.c_str(), &file_stats);
                pwrite(stats_file_fd, &file_stats, sizeof(struct stat), 0);
                fsync(stats_file_fd);
                close(stats_file_fd);
                // Return fd
                return new_file;
                // return reply.err();
            } else {
                close(stats_file_fd);
                unlink(stats_file_path);
                return -1;
            }
        } else if ((rpcbuf.st_mtim.tv_sec < file_stats.st_mtim.tv_sec) || ((rpcbuf.st_mtim.tv_sec == file_stats.st_mtim.tv_sec) && (rpcbuf.st_mtim.tv_nsec < file_stats.st_mtim.tv_nsec))){
            // Copy stats into temporary file
            local_res = lstat(path.c_str(), &file_stats);
            pwrite(stats_file_fd, &file_stats, sizeof(struct stat), 0);
            fsync(stats_file_fd);
            close(stats_file_fd);
            return open(path.c_str(), flags);
        }
        return -1;

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
        // std::cout<<"Creating File:"<<path<<std::endl;
        // request.set_mode(mode);

        struct stat rpcbuf;
        struct stat file_stats;
	    int local_res;

        // Check if the directory in which this file
        // supposedly resides exists on server.
        // If it doesn't, return.
        char * file_dirname = (char *) malloc(PATH_MAX);
        char* c_path = new char[path.length() + 1];
        strcpy(c_path, path.c_str());
        file_dirname = dirname(const_cast<char*>(c_path));
        int directory_exist = GetAttr(file_dirname, &rpcbuf);
            // std::cout<<"Checking if dir exists"<<file_dirname<<std::endl;
        if (directory_exist < 0) {
            return -errno;
        }

        // Dump file stats into temporary file on open
        char * stats_file_path = (char *) malloc(path.size() + 16);
        snprintf(stats_file_path, path.size() + 15, "%s.file_stats_tmp", path.c_str());

        // std::cout<<"After dir check"<<path<<std::endl;
        int ret = GetAttr(path, &rpcbuf);
        if (ret < 0){
            // std::cout<<"GetAttr return val < 0"<<path<<std::endl;
            mkpath(const_cast<char*>(path.c_str()), 777);
            // mkpath(const_cast<char*>(path.c_str()), mode);
            int rc = open(path.c_str(), flags, mode);
            if (rc == -1) {
                // std::cout<<"open threw an error"<<path<<std::endl;
                // fprintf(stdout, "open threw an error - it is %s\n", strerror(errno));
                return -errno;
            }
            // Copy stats into temporary file
            int stats_file_fd = open(stats_file_path, O_RDWR | O_CREAT, 0777);
            local_res = lstat(path.c_str(), &file_stats);
            pwrite(stats_file_fd, &file_stats, sizeof(struct stat), 0);
            fsync(stats_file_fd);
            close(stats_file_fd);
            return rc;
        }

        OpenReply reply;
        ClientContext context;
        local_res = lstat(path.c_str(), &file_stats);
        // std::cout << "local file stat value is " << local_res << " at path " << path << std::endl;
    	if ((local_res == -1) && (errno == ENOENT)) {
            // std::cout<<"File not found locally, but found on server"<<path<<std::endl;
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
                // Copy stats into temporary file
                int stats_file_fd = open(stats_file_path, O_RDWR | O_CREAT, 0777);
                local_res = lstat(path.c_str(), &file_stats);
                pwrite(stats_file_fd, &file_stats, sizeof(struct stat), 0);
                fsync(stats_file_fd);
                close(stats_file_fd);
		        // Return fd
		        return new_file;
                // return reply.err();
            } else {
                return -1;
            }
	    }
        return -1;
    }

    int Close(const std::string& path, int fd){
        CloseRequest request;
        request.set_path(path);
	std::cout<<"Closing File:"<<path<<std::endl;
	int rc, close_rc;

	rc = fsync(fd);
	if (rc == -1) {
            return -errno;
	}

	struct stat file_info;
	rc = fstat(fd, &file_info);
	if (rc == -1) {
            return -errno;
	}

	off_t file_size = file_info.st_size;
	char * buf = (char *) malloc(file_size - 1);
	int read_fd = open(path.c_str(), O_RDONLY);
	pread(read_fd, buf, file_size - 1, 0);
	// pread(fd, buf, file_size, 0);
	close(read_fd);
	close_rc = close(fd);

	if (close_rc == -1) {
            return -errno;
	}

	rc = lstat(path.c_str(), &file_info);
	if (rc == -1) {
            return -errno;
	}

	// Check temporary file with stats from open
	char * stats_file_path = (char *) malloc(path.size() + 16);
	snprintf(stats_file_path, path.size() + 15, "%s.file_stats_tmp", path.c_str());
	int stats_file_fd = open(stats_file_path, O_RDONLY);
	struct stat file_stats_at_open;
	pread(stats_file_fd, &file_stats_at_open, sizeof(struct stat), 0);
	close(stats_file_fd);

        struct stat server_stats;
        rc = GetAttr(path, &server_stats);

	// If the file has not been modified, no need to flush changes to server as long as file exists on server
	if (rc == 0) {
		if((file_info.st_mtim.tv_sec == file_stats_at_open.st_mtim.tv_sec) && (file_info.st_mtim.tv_nsec == file_stats_at_open.st_mtim.tv_nsec)) {
	                unlink(stats_file_path);
			return close_rc;
		}
	}

        request.set_path(path);
	request.set_file(std::string(buf, file_size - 1));
	request.set_num_bytes(file_size - 1);

        CloseReply reply;
        ClientContext context;
        Status status = stub_->Close(&context, request, &reply);
	if (status.ok()) {
            struct timespec updated_time[2];
            rc = lstat(path.c_str(), &file_info);
            updated_time[0] = file_info.st_atim;
            // struct stat server_stats;
            // rc = GetAttr(path, &server_stats);
            memcpy(&updated_time[1], (reply.m_tim()).data(), sizeof(struct timespec));
            utimensat(AT_FDCWD, path.c_str(), updated_time, 0);
            // futimens may not be needed
            // int time_fd = open(path.c_str(), O_RDWR);
            // futimens(time_fd, updated_time);
            // close(time_fd);
	}
	unlink(stats_file_path);
        return status.ok() ? reply.err() : -1;
    }

    int Unlink(const std::string& path) {
        UnlinkRequest request;
        request.set_path(path);

        UnlinkReply reply;
        ClientContext context;
        Status status = stub_->Unlink(&context, request, &reply);
        return status.ok() ? reply.err() : -1;
    }

    int Rename(const std::string& oldpath, const std::string& newpath) {
        RenameRequest request;
        request.set_oldpath(oldpath);
        request.set_newpath(newpath);

        RenameReply reply;
        ClientContext context;
        Status status = stub_->Rename(&context, request, &reply);

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

// long int Getxattr(UnreliableAFS* unreliableAFS, const char* path, const char* name, void* value, size_t size){
//   return unreliableAFS->GetXAttr(path, name, value, size);
// }

long int Getxattr(UnreliableAFS* unreliableAFS, const char* path, const char* name, void* value, size_t size){
  return unreliableAFS->GetXAttr(path, name, value, size);
}

int Access(UnreliableAFS* unreliableAFS, const char* path, int mode) {
    return unreliableAFS->Access(path, mode);
}

DIR* Opendir(UnreliableAFS* unreliableAFS, const char* path){
  return unreliableAFS->Opendir(path);
}

int Readdir(UnreliableAFS* unreliableAFS, const char* path, void* buf, fuse_fill_dir_t filler){
    return unreliableAFS->Readdir(path, buf, filler);
}

int Open(UnreliableAFS* unreliableAFS, const char* path, int flags){
  return unreliableAFS->Open(path, flags);
}

int OpenM(UnreliableAFS* unreliableAFS, const char* path, fuse_file_info* fi){
  return unreliableAFS->OpenM(path, fi);
}

int CloseM(UnreliableAFS* unreliableAFS, const std::string& path, fuse_file_info* fi) {
    return unreliableAFS->CloseM(path, fi);
}

// int WriteM(UnreliableAFS* unreliableAFS, const char* path, char* data, int size, int offset) {
//     return unreliableAFS->WriteM(path, data, size, offset);
// }

// int ReadM(UnreliableAFS* unreliableAFS, const char* path, char* data, int size, int offset) {
//     return unreliableAFS->ReadM(path, data, size, offset);
// }

int Create(UnreliableAFS* unreliableAFS, const char* path, int flags, int mode){
  return unreliableAFS->Create(path, flags, mode);
}

int Close(UnreliableAFS* unreliableAFS, const char* path, int fd){
  return unreliableAFS->Close(path, fd);
}

int Unlink(UnreliableAFS* unreliableAFS, const char* path){
  return unreliableAFS->Unlink(path);
}

int Rename(UnreliableAFS* unreliableAFS, const char* old_path, const char* new_path){
  return unreliableAFS->Rename(old_path, new_path);
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
