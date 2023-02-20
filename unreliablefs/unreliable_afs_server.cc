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
using unreliable_afs::ReadDirRequest;
using unreliable_afs::OpenDirRequest;
using unreliable_afs::OpenDirReply;
using unreliable_afs::OpenRequest;
using unreliable_afs::OpenReply;
using unreliable_afs::CloseRequest;
using unreliable_afs::CloseReply;
using unreliable_afs::UnreliableAFSProto;

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
            int stat_size = sizeof(struct stat*);
            buf.resize(stat_size);
            assert(buf.size() == sizeof(struct stat*));
            memcpy(&buf[0], &stbuf, buf.size());
            reply->set_buf(buf);    
            reply->set_err(res);
            return Status::OK;
        }

    //     Status GetXAttr(ServerContext* context, const GetXAttrRequest* request,
    //             GetXAttrReply* reply) override {
    //         int res;
    //         struct stat stbuf;
    //         std::string path = server_base_directory + request->path();
    //         printf("GetXAttr: %s \n", path.c_str());

	  //   char * value = (char *) malloc(request->size());
    //         ssize_t xattr_size = getxattr(path.c_str(), request->name().c_str(), value, request->size());
    //         if (xattr_size == -1) {
    //             reply->set_size(-errno);
		// // reply->set_value(std::string(value, request->size)); // Set as optional, so shouldn't have to be sent here
    //             return Status::OK;
    //         }
    //         reply->set_value(std::string(value, xattr_size));
	  //   reply->set_size(xattr_size);
    //         return Status::OK;
    //     }

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
            buf.resize(sizeof(dp));
            memcpy(&buf[0], dp, buf.size());
            reply->set_dir(buf);
            return Status::OK;
        }

        Status ReadDir(ServerContext* context, const ReadDirRequest* request, ServerWriter<ReadDirReply>* writer) override {
            ReadDirReply* reply = new ReadDirReply();
            // default errno = 0
            reply->set_err(0);

            std::string path = server_base_directory + request->path();
            printf("ReadDir: %s \n", path.c_str());

            DIR *dp;
            struct dirent *de;

            dp = opendir(path.c_str());
            if (dp == NULL) {
                reply->set_err(-errno);
                return Status::OK;
            }
            while ((de = readdir(dp)) != NULL) {
                std::string buf;
                buf.resize(sizeof(struct dirent));
                memcpy(&buf[0], de, buf.size());
                reply->set_buf(buf);
                writer->Write(*reply);
            }
            closedir(dp);
            return Status::OK;
        }

        Status Open(ServerContext* context, const OpenRequest* request,
                OpenReply* reply) override {
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            std::cout<<"Opening File:"<<path<<std::endl;
            int res;

            res = open(path.c_str(), request->flags());
            //res = open(path.c_str(), request->flags(), request->mode());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }

	    struct stat file_info;
	    fstat(res, &file_info);
	    off_t file_size = file_info.st_size;
	    char * buf = (char *) malloc(file_size);
	    pread(res, buf, file_size, 0);
	    close(res);

	    reply->set_err(0);
	    reply->set_file(std::string(buf, file_size));
	    reply->set_num_bytes(file_size);

	    return Status::OK;
        }

        Status Close(ServerContext* context, const CloseRequest* request,
                CloseReply* reply) override {
            reply->set_err(0);
            std::string path = server_base_directory + request->path();
            std::cout<<"Server closing File:"<<path<<std::endl;
            int res;

	    // Check if directory we're writing to exists
	    char * file_dirname = (char *) malloc(PATH_MAX);
	    file_dirname = dirname(const_cast<char*>(path.c_str()));
	    struct stat dir_stats;
	    res = lstat(file_dirname, &dir_stats);
	    if (res == -1) {
		reply->set_err(-errno);
                return Status::OK;
	    }

	    struct stat file_stats;
	    res = lstat(path.c_str(), &file_stats);
	    if ((res == -1) && (errno == ENOENT)){
		int num_bytes = request->num_bytes();
		char* fetched_file = (char *) malloc(num_bytes);
		memcpy(fetched_file, (char *)request->file().data(), num_bytes);
		int new_file = open(path.c_str(), O_RDWR | O_CREAT, 0777);
		// int new_file = open(path.c_str(), O_RDWR | O_CREAT, 0664);
		write(new_file, fetched_file, num_bytes);
		lseek(new_file, SEEK_SET, 0);
		fsync(new_file);
		res = close(new_file);
            } else if (res == 0) {
		// Create a temporary file
		char * tmp_path = (char *) malloc(path.size() + 8);
		snprintf(tmp_path, path.size() + 7, "%s.tmpbak", path.c_str());
		int num_bytes = request->num_bytes();
		char* fetched_file = (char *) malloc(num_bytes);
		memcpy(fetched_file, (char *)request->file().data(), num_bytes);
		int new_file = open(tmp_path, O_RDWR | O_CREAT, 0777);
		// int new_file = open(tmp_path, O_RDWR | O_CREAT, 0664);
		write(new_file, fetched_file, num_bytes);
		lseek(new_file, SEEK_SET, 0);
		fsync(new_file);
		res = close(new_file);
		unlink(path.c_str());
		rename(tmp_path, path.c_str());
	    } else {
		reply->set_err(-errno);
                return Status::OK;
	    }

	    if (res == -1) {
                reply->set_err(-errno);
	    } else {
                reply->set_err(0);
	    }
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
