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
#include <dirent.h>

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
            int stat_size = sizeof(struct stat);
            buf.resize(stat_size);
            assert(buf.size() == sizeof(struct stat));
            memcpy(&buf[0], &stbuf, buf.size());
            reply->set_buf(buf);    
            reply->set_err(res);
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