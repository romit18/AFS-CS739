// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: unreliable_afs.proto

#include "unreliable_afs.pb.h"
#include "unreliable_afs.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/codegen/channel_interface.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/rpc_service_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/sync_stream.h>
namespace unreliable_afs {

static const char* UnreliableAFS_method_names[] = {
  "/unreliable_afs.UnreliableAFS/MkDir",
};

std::unique_ptr< UnreliableAFS::Stub> UnreliableAFS::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< UnreliableAFS::Stub> stub(new UnreliableAFS::Stub(channel, options));
  return stub;
}

UnreliableAFS::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_MkDir_(UnreliableAFS_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status UnreliableAFS::Stub::MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::unreliable_afs::MkDirReply* response) {
  return ::grpc::internal::BlockingUnaryCall< ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_MkDir_, context, request, response);
}

void UnreliableAFS::Stub::async::MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_MkDir_, context, request, response, std::move(f));
}

void UnreliableAFS::Stub::async::MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_MkDir_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>* UnreliableAFS::Stub::PrepareAsyncMkDirRaw(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::unreliable_afs::MkDirReply, ::unreliable_afs::MkDirReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_MkDir_, context, request);
}

::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>* UnreliableAFS::Stub::AsyncMkDirRaw(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncMkDirRaw(context, request, cq);
  result->StartCall();
  return result;
}

UnreliableAFS::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      UnreliableAFS_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< UnreliableAFS::Service, ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](UnreliableAFS::Service* service,
             ::grpc::ServerContext* ctx,
             const ::unreliable_afs::MkDirReq* req,
             ::unreliable_afs::MkDirReply* resp) {
               return service->MkDir(ctx, req, resp);
             }, this)));
}

UnreliableAFS::Service::~Service() {
}

::grpc::Status UnreliableAFS::Service::MkDir(::grpc::ServerContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace unreliable_afs

