#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "unreliable_afs.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::ClientWriter;
using grpc::Status;

using unreliable_afs::UnreliableAFS;
using unreliable_afs::MkDirReq;
using unreliable_afs::MkDirReply;

class GreeterClient {

    public:
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(UnreliableAFS::NewStub(channel)) {}


    int MkDir(const std::string& path, int mode) {
        MkDirReq request;
        request.set_path(path);
        request.set_mode(mode);

        MkDirReply reply;
        ClientContext context;
        Status status = stub_->MkDir(&context, request, &reply);

        return status.ok() ? reply.err() : -1;
    }



    private:
    std::unique_ptr<UnreliableAFS::Stub> stub_;
    
};


int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        target_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --target="
                  << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  } else {
    target_str = "localhost:50051";
  }
  GreeterClient greeter(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string dirpath("/world");
  int res = greeter.MkDir(dirpath, 0777);
  std::cout << "Greeter received: " << res << std::endl;


  return 0;
}