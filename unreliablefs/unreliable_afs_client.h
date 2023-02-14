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
using unreliable_afs::MkdirRequest;
using unreliable_afs::MkdirReply;

class GreeterClient {

    public:
    GreeterClient(std::shared_ptr<Channel> channel)
        : stub_(UnreliableAFS::NewStub(channel)) {}


    int Mkdir(const std::string& path, int mode) {
        MkdirRequest request;
        request.set_path(path);
        request.set_mode(mode);

        MkdirReply reply;
        ClientContext context;
        Status status = stub_->Mkdir(&context, request, &reply);

        return status.ok() ? reply.err() : -1;
    }



    private:
    std::unique_ptr<UnreliableAFS::Stub> stub_;
    
};
