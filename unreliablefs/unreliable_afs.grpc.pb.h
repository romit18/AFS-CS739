// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: unreliable_afs.proto
#ifndef GRPC_unreliable_5fafs_2eproto__INCLUDED
#define GRPC_unreliable_5fafs_2eproto__INCLUDED

#include "unreliable_afs.pb.h"

#include <functional>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace unreliable_afs {

class UnreliableAFS final {
 public:
  static constexpr char const* service_full_name() {
    return "unreliable_afs.UnreliableAFS";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    // rpc GetAttr (GetAttrReq) returns (GetAttrReply) {}
    virtual ::grpc::Status MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::unreliable_afs::MkDirReply* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::unreliable_afs::MkDirReply>> AsyncMkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::unreliable_afs::MkDirReply>>(AsyncMkDirRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::unreliable_afs::MkDirReply>> PrepareAsyncMkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::unreliable_afs::MkDirReply>>(PrepareAsyncMkDirRaw(context, request, cq));
    }
    class async_interface {
     public:
      virtual ~async_interface() {}
      // rpc GetAttr (GetAttrReq) returns (GetAttrReply) {}
      virtual void MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response, std::function<void(::grpc::Status)>) = 0;
      virtual void MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response, ::grpc::ClientUnaryReactor* reactor) = 0;
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::unreliable_afs::MkDirReply>* AsyncMkDirRaw(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::unreliable_afs::MkDirReply>* PrepareAsyncMkDirRaw(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::unreliable_afs::MkDirReply* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>> AsyncMkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>>(AsyncMkDirRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>> PrepareAsyncMkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>>(PrepareAsyncMkDirRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response, std::function<void(::grpc::Status)>) override;
      void MkDir(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response, ::grpc::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>* AsyncMkDirRaw(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::unreliable_afs::MkDirReply>* PrepareAsyncMkDirRaw(::grpc::ClientContext* context, const ::unreliable_afs::MkDirReq& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_MkDir_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    // rpc GetAttr (GetAttrReq) returns (GetAttrReply) {}
    virtual ::grpc::Status MkDir(::grpc::ServerContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_MkDir : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_MkDir() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_MkDir() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MkDir(::grpc::ServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestMkDir(::grpc::ServerContext* context, ::unreliable_afs::MkDirReq* request, ::grpc::ServerAsyncResponseWriter< ::unreliable_afs::MkDirReply>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_MkDir<Service > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_MkDir : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_MkDir() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::unreliable_afs::MkDirReq* request, ::unreliable_afs::MkDirReply* response) { return this->MkDir(context, request, response); }));}
    void SetMessageAllocatorFor_MkDir(
        ::grpc::MessageAllocator< ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_MkDir() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MkDir(::grpc::ServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* MkDir(
      ::grpc::CallbackServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_MkDir<Service > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_MkDir : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_MkDir() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_MkDir() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MkDir(::grpc::ServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_MkDir : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_MkDir() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_MkDir() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MkDir(::grpc::ServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestMkDir(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_MkDir : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_MkDir() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->MkDir(context, request, response); }));
    }
    ~WithRawCallbackMethod_MkDir() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status MkDir(::grpc::ServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* MkDir(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_MkDir : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_MkDir() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::unreliable_afs::MkDirReq, ::unreliable_afs::MkDirReply>* streamer) {
                       return this->StreamedMkDir(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_MkDir() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status MkDir(::grpc::ServerContext* /*context*/, const ::unreliable_afs::MkDirReq* /*request*/, ::unreliable_afs::MkDirReply* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedMkDir(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::unreliable_afs::MkDirReq,::unreliable_afs::MkDirReply>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_MkDir<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_MkDir<Service > StreamedService;
};

}  // namespace unreliable_afs


#endif  // GRPC_unreliable_5fafs_2eproto__INCLUDED
