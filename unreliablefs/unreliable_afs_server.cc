#include <iostream>
#include <cassert>
#include <memory>
#include <string>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/xattr.h>
#include <sys/xattr.h>

#include <grpc++/grpc++.h>

#include "unreliable_afs.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::ServerReader;
using grpc::Status;


using unreliable_afs::MkdirRequest;
using unreliable_afs::MkdirReply;
using unreliable_afs::RmdirRequest;
using unreliable_afs::RmdirReply;
using unreliable_afs::GetAttrRequest;
using unreliable_afs::GetAttrReply;
using unreliable_afs::GetXAttrRequest;
using unreliable_afs::GetXAttrReply;
using unreliable_afs::ReadDirRequest;
using unreliable_afs::ReadDirReply;
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
using unreliable_afs::UnreliableAFSProto;
using unreliable_afs::AccessRequest;
using unreliable_afs::AccessReply;

std::string server_base_directory;

class UnreliableAFSServiceImpl final : public UnreliableAFSProto::Service {
    public:
        UnreliableAFSServiceImpl() {
        }

        Status Mkdir(ServerContext* context, const MkdirRequest* request,
                MkdirReply* reply) override {
            // default errno = 0
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            std::cout<<"Making Directory:"<<path<<std::endl;
            int res;

            res = mkdir(path.c_str(), request->mode());
            std::cout<<"Making result:"<<res<<std::endl;
            std::cout<<"Request path:"<<request->path()<<std::endl;
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }
            reply->set_err(res);
            return Status::OK;
        }

        Status Rmdir(ServerContext* context, const RmdirRequest* request,
                RmdirReply* reply) override {
            // default errno = 0
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            printf("Removing Directory: %s \n", path.c_str());
            int res;

            res = rmdir(path.c_str());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }

            reply->set_err(res);
            return Status::OK;
        }

        Status GetAttr(ServerContext* context, const GetAttrRequest* request,
                GetAttrReply* reply) override {
            // default errno = 0
            reply->set_err(0);
            int res;
            struct stat stbuf;
            std::string path = server_base_directory + request->path();
            printf("GetAttr: %s \n", path.c_str());

            res = lstat(path.c_str(), &stbuf);
            if (res < 0) {
                reply->set_err(-errno);
                return Status::OK;
            } 
            std::string buf;
            int stat_size = sizeof(struct stat);
            buf.resize(stat_size);
            assert(buf.size() == sizeof(struct stat));
            memcpy(&buf[0], &stbuf, buf.size());
            reply->set_buf(buf);    
            reply->set_err(res);
            return Status::OK;
        }

        Status GetXAttr(ServerContext* context, const GetXAttrRequest* request,
                GetXAttrReply* reply) override {
            // int res;
            struct stat stbuf;
            std::string path = server_base_directory + request->path();
            printf("GetXAttr: %s \n", path.c_str());

	    char * value = (char *) malloc(request->size());
            ssize_t xattr_size = getxattr(path.c_str(), request->name().c_str(), value, request->size());
            if (xattr_size == -1) {
                reply->set_size(-errno);
		// reply->set_value(std::string(value, request->size)); // Set as optional, so shouldn't have to be sent here
                return Status::OK;
            }
            reply->set_value(std::string(value, xattr_size));
	    reply->set_size(xattr_size);
            return Status::OK;
        }


        Status Access(ServerContext* context, const AccessRequest* request, AccessReply* reply) override {
            reply->set_err(0);

            std::string path = server_base_directory + request->path();
            printf("Access: %s \n", path.c_str());

            int res;

            res = access(path.c_str(), request->mode());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }
            reply->set_err(res);
            return Status::OK;
        }

        Status OpenDir(ServerContext* context, const OpenDirRequest* request,
                OpenDirReply* reply) override {
            // default errno = 0
            reply->set_err(0);

            std::string path = server_base_directory + request->path();
            printf("OpenDir: %s \n", path.c_str());

            DIR *dp;

            dp = opendir(path.c_str());
          
            if (dp == NULL) {
                reply->set_err(-errno);
                return Status::OK;
            }
            std::string buf;
            buf.resize(65536);
            memcpy(&buf[0], dp, buf.size());
            reply->set_dir(buf);
            return Status::OK;
        }

        Status ReadDir(ServerContext* context, const ReadDirRequest* request, ServerWriter<ReadDirReply>* writer) override {
            ReadDirReply reply;
            // char path[PATH_MAX];
            // path[0] = '\0';

            // strncat(path, server_base_directory, PATH_MAX);
            // strncat(path, (request->path()).c_str(), PATH_MAX);

            
            std::string path = server_base_directory + request->path();

            DIR *dp;
            struct dirent *de;

            dp = opendir(path.c_str());
            if (dp == NULL) {
                reply.set_err(-1);
                writer->Write(reply);
                return Status::OK;
            }

            de = readdir(dp);
            if (de == 0) {
                reply.set_err(-1);
                writer->Write(reply);
                return Status::OK;
            }


            do {
                reply.set_err(0);
                reply.set_name(std::string(de->d_name));
                writer->Write(reply);
            } while ((de = readdir(dp)) != NULL);

            return Status::OK;

            // default errno = 0
            // reply->set_err(0);

            // std::string path = server_base_directory + request->path();
            // printf("ReadDir: %s \n", path.c_str());

            // DIR *dp;
            // struct dirent *de;

            // dp = opendir(path.c_str());
            // if (dp == NULL) {
            //     reply->set_err(-errno);
            //     return Status::OK;
            // }
            // while ((de = readdir(dp)) != NULL) {
            //     std::string buf;
            //     buf.resize(sizeof(struct dirent));
            //     memcpy(&buf[0], de, buf.size());
            //     reply->set_buf(buf);
            //     writer->Write(*reply);
            // }
            // closedir(dp);
            // return Status::OK;
        }

        Status Open(ServerContext* context, const OpenRequest* request,
                OpenReply* reply) override {
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            std::cout<<"Opening File:"<<path<<std::endl;
            int res;

            res = open(path.c_str(), O_RDWR);
            // res = open(path.c_str(), request->flags());
            // res = open(path.c_str(), request->flags(), request->mode());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }

	    struct stat file_info;
	    fstat(res, &file_info);
	    off_t file_size = file_info.st_size;
	    char * buf = (char *) malloc(file_size - 1);
	    pread(res, buf, file_size - 1, 0);
	    close(res);

	    reply->set_err(0);
	    reply->set_file(std::string(buf, file_size - 1));
	    reply->set_num_bytes(file_size - 1);

	    return Status::OK;
        }

        Status Close(ServerContext* context, const CloseRequest* request,
                CloseReply* reply) override {
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            std::cout<<"Server closing File:"<<path<<std::endl;
            int rc;
	    struct stat dir_stats, file_stats;

	    // Check if directory we're writing to exists
	    char * file_dirname = (char *) malloc(PATH_MAX);
	    char * c_path = new char[path.length() + 1];
	    strcpy(c_path, path.c_str());
	    file_dirname = dirname(const_cast<char*>(c_path));
	    rc = lstat(file_dirname, &dir_stats);
	    if (rc == -1) {
		reply->set_err(-errno);
                return Status::OK;
	    }
	    // std::cout << "Directory exists" << std::endl;

	    rc = lstat(path.c_str(), &file_stats);
	    if ((rc == -1) && (errno == ENOENT)){
	        // std::cout << "new file" << std::endl;
		int num_bytes = request->num_bytes();
		char * fetched_file = (char *) malloc(num_bytes);
		memcpy(fetched_file, (char *)request->file().data(), num_bytes);
		int new_file = open(path.c_str(), O_RDWR | O_CREAT, 0777);
		// int new_file = open(path.c_str(), O_RDWR | O_CREAT, 0664);
		write(new_file, fetched_file, num_bytes);
		lseek(new_file, SEEK_SET, 0);
		fsync(new_file);
		rc = close(new_file);
		rc = lstat(path.c_str(), &file_stats);
		// Extract modified time and send it
		std::string time_buf;
		time_buf.resize(sizeof(struct timespec));
                assert(time_buf.size() == sizeof(struct timespec));
		struct timespec modified_time = file_stats.st_mtim;
		memcpy(&time_buf[0], &modified_time, time_buf.size());
		reply->set_m_tim(time_buf);
            } else if (rc == 0) {
		// Create a temporary file
	        // std::cout << "existing file" << std::endl;
		char * tmp_path = (char *) malloc(path.size() + 8);
		snprintf(tmp_path, path.size() + 7, "%s.tmpbak", path.c_str());
		int num_bytes = request->num_bytes();
		char * fetched_file = (char *) malloc(num_bytes);
		memcpy(fetched_file, (char *)request->file().data(), num_bytes);
		int new_file = open(tmp_path, O_RDWR | O_CREAT, 0777);
		// int new_file = open(tmp_path, O_RDWR | O_CREAT, 0664);
		write(new_file, fetched_file, num_bytes);
		lseek(new_file, SEEK_SET, 0);
		fsync(new_file);
		rc = close(new_file);
		unlink(path.c_str());
		rename(tmp_path, path.c_str());
		// Extract modified time and send it
		std::string time_buf;
		time_buf.resize(sizeof(struct timespec));
                assert(time_buf.size() == sizeof(struct timespec));
		struct timespec modified_time = file_stats.st_mtim;
		memcpy(&time_buf[0], &modified_time, time_buf.size());
		reply->set_m_tim(time_buf);
	    } else {
		reply->set_err(-errno);
                return Status::CANCELLED;
	    }

	    if (rc == -1) {
                reply->set_err(-errno);
	    } else {
                reply->set_err(0);
	    }
            return Status::OK;
        }

        Status Unlink(ServerContext* context, const UnlinkRequest* request,
                UnlinkReply* reply) override {
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            printf("Removing File: %s \n", path.c_str());
            int res;

            res = unlink(path.c_str());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }

            reply->set_err(res);
            return Status::OK;
        }

        Status Rename(ServerContext* context, const RenameRequest* request,
                RenameReply* reply) override {
            reply->set_err(0);
            std::string oldpath = server_base_directory + request->oldpath();
            std::string newpath = server_base_directory + request->newpath();
            printf("Renaming File: %s to %s \n", oldpath.c_str(), newpath.c_str());
            int res;

            res = rename(oldpath.c_str(), newpath.c_str());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }

            reply->set_err(res);
            return Status::OK;
        }

        Status OpenM(ServerContext* context, const OpenMRequest* request,
                OpenMReply* reply) override {
            // default errno = 0
            reply->set_err(0);

            std::string path = server_base_directory + request->path();
            printf("Open: %s \n", path.c_str());

            int res;

            res = open(path.c_str(), request->flag()); 
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }
            close(res);
            reply->set_err(res);
            return Status::OK;
        }

        Status ReadM(ServerContext* context, const ReadMRequest* request,
                ServerWriter<ReadMReply>* writer) override {
            ReadMReply* reply = new ReadMReply();


            reply->set_num_bytes(0);
            int res;
            std::string path = server_base_directory + request->path();
            printf("ReadM: %s \n", path.c_str());
            int size = request->size();
            int offset = request->offset();

            int fd = open(path.c_str(), O_RDONLY);
            if (fd == -1) {
                reply->set_num_bytes(-1);
                return Status::OK;
            }

            std::string buf;
            buf.resize(size);
            
            int b = pread(fd, &buf[0], size, offset);
            if (b != size) {
                printf("ReadM: PREAD didn't read %d bytes from offset %d\n", size, offset);
            } 
            if (b == -1) {
                reply->set_num_bytes(-errno);
            }
            close(fd);

            int remain = b;
            int stump = 1048576; // 1Mb
            int curr = 0;
            
            while (remain > 0) {
                reply->set_buf(buf.substr(curr, std::min(stump, remain)));
                reply->set_num_bytes(std::min(stump, remain));
                curr += stump;
                remain -= stump;
                writer->Write(*reply);
            }
            return Status::OK;
        }

        Status WriteM(ServerContext* context, ServerReader<WriteMRequest>* reader,
                     WriteMReply* reply) override { 
            std::string path;
            WriteMRequest request;
            int fd = -1;
            int res;
            int size;
            int offset;
            int num_bytes = 0;
            reply->set_num_bytes(num_bytes);
            while (reader->Read(&request)) {
                path = server_base_directory + request.path();
                size = request.size();
                offset = request.offset();
                std::string buf = request.buf();
                if (num_bytes == 0) {
                    fd = open(path.c_str(), O_WRONLY);
                    if (fd == -1) {
                        reply->set_num_bytes(-errno);
                        return Status::OK;
                    }
                    printf("WriteS: %s \n", path.c_str());
                }
                res = pwrite(fd, &buf[0], size, offset);
                // pwrite returns -1 when error, and store type in errno
                if (res == -1) {
                    reply->set_num_bytes(-errno);
                    return Status::OK;
                }
                num_bytes += res;
            }
            if (fd > 0) {
                fsync(fd);
                close(fd);
            }
            reply->set_num_bytes(num_bytes);
            return Status::OK;
        }


};

void RunServer(std::string base_path_str) {
  std::string server_address("0.0.0.0:50051");
  UnreliableAFSServiceImpl service;

  server_base_directory = base_path_str;

  grpc::EnableDefaultHealthCheckService(true);
//   grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {

  std::string base_path_str;
  std::string arg_str("--base");
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        base_path_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --base="
                  << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --base=" << std::endl;
      return 0;
    }
  } else {
    base_path_str = "/tmp/uafs";
  }

  RunServer(base_path_str);

  return 0;
}
