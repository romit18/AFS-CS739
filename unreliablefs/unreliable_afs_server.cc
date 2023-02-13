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


using unreliable_afs::MkDirReq;
using unreliable_afs::MkDirReply;
using unreliable_afs::UnreliableAFS;

class UnreliableAFSServiceImpl final : public UnreliableAFS::Service {
    public:
        const std::string path_prefix; 
        UnreliableAFSServiceImpl(): path_prefix("/tmp/uafs") {
        }

        Status MkDir(ServerContext* context, const MkDirReq* request,
                MkDirReply* reply) override {
            // default errno = 0
            reply->set_err(0);
            std::string path = path_prefix + request->path();
            printf("MkDir: %s \n", path.c_str());
            int res;

            res = mkdir(path.c_str(), request->mode());
            if (res == -1) {
                reply->set_err(-errno);
                return Status::OK;
            }
            reply->set_err(res);
            return Status::OK;
        }

};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  UnreliableAFSServiceImpl service;

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
  RunServer();

  return 0;
}