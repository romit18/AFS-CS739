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

using unreliable_afs::UnreliableAFSProto;
using unreliable_afs::MkdirRequest;
using unreliable_afs::MkdirReply;
using unreliable_afs::RmdirRequest;
using unreliable_afs::RmdirReply;
using unreliable_afs::GetAttrRequest;
using unreliable_afs::GetAttrReply;

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

    
    int GetAttr(const std::string& path, std::string buf){
        GetAttrRequest request;
        request.set_path(path);

        GetAttrReply reply;
        ClientContext context;
        Status status = stub_->GetAttr(&context, request, &reply);
        if (status.ok()) {
            buf = reply.buf();
            return reply.err();
        } else {
            return -1;
        }
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

int Getattr(UnreliableAFS* unreliableAFS, const char* path, const char* stbuf){
  return unreliableAFS->GetAttr(path, stbuf);
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