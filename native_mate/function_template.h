// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.

#ifndef NATIVE_MATE_FUNCTION_TEMPLATE_H_
#define NATIVE_MATE_FUNCTION_TEMPLATE_H_

#include "base/callback.h"
#include "base/logging.h"
#include "native_mate/arguments.h"
#include "native_mate/compat.h"
#include "native_mate/converter.h"
#include "v8/include/v8.h"

namespace mate {

class PerIsolateData;

enum CreateFunctionTemplateFlags {
  HolderIsFirstArgument = 1 << 0,
};

namespace internal {

template<typename T>
struct CallbackParamTraits {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T&> {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T*> {
  typedef T* LocalType;
};


// CallbackHolder and CallbackHolderBase are used to pass a base::Callback from
// CreateFunctionTemplate through v8 (via v8::FunctionTemplate) to
// DispatchToCallback, where it is invoked.

// This simple base class is used so that we can share a single object template
// among every CallbackHolder instance.
class CallbackHolderBase {
 public:
  v8::Handle<v8::External> GetHandle(v8::Isolate* isolate);

 protected:
  explicit CallbackHolderBase(v8::Isolate* isolate);
  virtual ~CallbackHolderBase();

 private:
  static MATE_WEAK_CALLBACK(WeakCallback, v8::External, CallbackHolderBase);

  v8::Persistent<v8::External> v8_ref_;

  DISALLOW_COPY_AND_ASSIGN(CallbackHolderBase);
};

template<typename Sig>
class CallbackHolder : public CallbackHolderBase {
 public:
  CallbackHolder(v8::Isolate* isolate,
                 const base::Callback<Sig>& callback,
                 int flags)
      : CallbackHolderBase(isolate), callback(callback), flags(flags) {}
  base::Callback<Sig> callback;
  int flags;
 private:
  virtual ~CallbackHolder() {}

  DISALLOW_COPY_AND_ASSIGN(CallbackHolder);
};


// This set of templates invokes a base::Callback, converts the return type to a
// JavaScript value, and returns that value to script via the provided
// mate::Arguments object.
//
// In C++, you can declare the function foo(void), but you can't pass a void
// expression to foo. As a result, we must specialize the case of Callbacks that
// have the void return type.
template<typename R, typename P1 = void, typename P2 = void,
    typename P3 = void, typename P4 = void, typename P5 = void,
    typename P6 = void>
struct Invoker {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R(P1, P2, P3, P4, P5, P6)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3,
      const P4& a4,
      const P5& a5,
      const P6& a6) {
    MATE_METHOD_RETURN(callback.Run(a1, a2, a3, a4, a5, a6));
  }
};
template<typename P1, typename P2, typename P3, typename P4, typename P5,
    typename P6>
struct Invoker<void, P1, P2, P3, P4, P5, P6> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void(P1, P2, P3, P4, P5, P6)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3,
      const P4& a4,
      const P5& a5,
      const P6& a6) {
    callback.Run(a1, a2, a3, a4, a5, a6);
    MATE_METHOD_RETURN_UNDEFINED();
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4,
    typename P5>
struct Invoker<R, P1, P2, P3, P4, P5, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R(P1, P2, P3, P4, P5)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3,
      const P4& a4,
      const P5& a5) {
    MATE_METHOD_RETURN(callback.Run(a1, a2, a3, a4, a5));
  }
};
template<typename P1, typename P2, typename P3, typename P4, typename P5>
struct Invoker<void, P1, P2, P3, P4, P5, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void(P1, P2, P3, P4, P5)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3,
      const P4& a4,
      const P5& a5) {
    callback.Run(a1, a2, a3, a4, a5);
    MATE_METHOD_RETURN_UNDEFINED();
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct Invoker<R, P1, P2, P3, P4, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R(P1, P2, P3, P4)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3,
      const P4& a4) {
    MATE_METHOD_RETURN(callback.Run(a1, a2, a3, a4));
  }
};
template<typename P1, typename P2, typename P3, typename P4>
struct Invoker<void, P1, P2, P3, P4, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void(P1, P2, P3, P4)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3,
      const P4& a4) {
    callback.Run(a1, a2, a3, a4);
    MATE_METHOD_RETURN_UNDEFINED();
  }
};

template<typename R, typename P1, typename P2, typename P3>
struct Invoker<R, P1, P2, P3, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R(P1, P2, P3)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3) {
    MATE_METHOD_RETURN(callback.Run(a1, a2, a3));
  }
};
template<typename P1, typename P2, typename P3>
struct Invoker<void, P1, P2, P3, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void(P1, P2, P3)>& callback,
      const P1& a1,
      const P2& a2,
      const P3& a3) {
    callback.Run(a1, a2, a3);
    MATE_METHOD_RETURN_UNDEFINED();
  }
};

template<typename R, typename P1, typename P2>
struct Invoker<R, P1, P2, void, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R(P1, P2)>& callback,
      const P1& a1,
      const P2& a2) {
    MATE_METHOD_RETURN(callback.Run(a1, a2));
  }
};
template<typename P1, typename P2>
struct Invoker<void, P1, P2, void, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void(P1, P2)>& callback,
      const P1& a1,
      const P2& a2) {
    callback.Run(a1, a2);
    MATE_METHOD_RETURN_UNDEFINED();
  }
};

template<typename R, typename P1>
struct Invoker<R, P1, void, void, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R(P1)>& callback,
      const P1& a1) {
    MATE_METHOD_RETURN(callback.Run(a1));
  }
};
template<typename P1>
struct Invoker<void, P1, void, void, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void(P1)>& callback,
      const P1& a1) {
    callback.Run(a1);
    MATE_METHOD_RETURN_UNDEFINED();
  }
};

template<typename R>
struct Invoker<R, void, void, void, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<R()>& callback) {
    MATE_METHOD_RETURN(callback.Run());
  }
};
template<>
struct Invoker<void, void, void, void, void, void, void> {
  inline static MATE_METHOD_RETURN_TYPE Go(
      Arguments& args,
      const base::Callback<void()>& callback) {
    callback.Run();
    MATE_METHOD_RETURN_UNDEFINED();
  }
};


template<typename T>
bool GetNextArgument(Arguments* args, int create_flags, bool is_first,
                     T* result) {
  if (is_first && (create_flags & HolderIsFirstArgument) != 0) {
    return args->GetHolder(result);
  } else {
    return args->GetNext(result);
  }
}

// For advanced use cases, we allow callers to request the unparsed Arguments
// object and poke around in it directly.
inline bool GetNextArgument(Arguments* args, int create_flags, bool is_first,
                            Arguments* result) {
  *result = *args;
  return true;
}
inline bool GetNextArgument(Arguments* args, int create_flags, bool is_first,
                            Arguments** result) {
  *result = args;
  return true;
}

// It's common for clients to just need the isolate, so we make that easy.
inline bool GetNextArgument(Arguments* args, int create_flags,
                            bool is_first, v8::Isolate** result) {
  *result = args->isolate();
  return true;
}


// DispatchToCallback converts all the JavaScript arguments to C++ types and
// invokes the base::Callback.
template<typename Sig>
struct Dispatcher {
};

template<typename R>
struct Dispatcher<R()> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R()> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    return Invoker<R>::Go(args, holder->callback);
  }
};

template<typename R, typename P1>
struct Dispatcher<R(P1)> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R(P1)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    typename CallbackParamTraits<P1>::LocalType a1;
    if (!GetNextArgument(&args, holder->flags, true, &a1)) {
      args.ThrowError();
      MATE_METHOD_RETURN_UNDEFINED();
    }

    return Invoker<R, P1>::Go(args, holder->callback, a1);
  }
};

template<typename R, typename P1, typename P2>
struct Dispatcher<R(P1, P2)> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R(P1, P2)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    typename CallbackParamTraits<P1>::LocalType a1;
    typename CallbackParamTraits<P2>::LocalType a2;
    if (!GetNextArgument(&args, holder->flags, true, &a1) ||
        !GetNextArgument(&args, holder->flags, false, &a2)) {
      args.ThrowError();
      MATE_METHOD_RETURN_UNDEFINED();
    }

    return Invoker<R, P1, P2>::Go(args, holder->callback, a1, a2);
  }
};

template<typename R, typename P1, typename P2, typename P3>
struct Dispatcher<R(P1, P2, P3)> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R(P1, P2, P3)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    typename CallbackParamTraits<P1>::LocalType a1;
    typename CallbackParamTraits<P2>::LocalType a2;
    typename CallbackParamTraits<P3>::LocalType a3;
    if (!GetNextArgument(&args, holder->flags, true, &a1) ||
        !GetNextArgument(&args, holder->flags, false, &a2) ||
        !GetNextArgument(&args, holder->flags, false, &a3)) {
      args.ThrowError();
      MATE_METHOD_RETURN_UNDEFINED();
    }

    return Invoker<R, P1, P2, P3>::Go(args, holder->callback, a1, a2, a3);
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct Dispatcher<R(P1, P2, P3, P4)> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R(P1, P2, P3, P4)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    typename CallbackParamTraits<P1>::LocalType a1;
    typename CallbackParamTraits<P2>::LocalType a2;
    typename CallbackParamTraits<P3>::LocalType a3;
    typename CallbackParamTraits<P4>::LocalType a4;
    if (!GetNextArgument(&args, holder->flags, true, &a1) ||
        !GetNextArgument(&args, holder->flags, false, &a2) ||
        !GetNextArgument(&args, holder->flags, false, &a3) ||
        !GetNextArgument(&args, holder->flags, false, &a4)) {
      args.ThrowError();
      MATE_METHOD_RETURN_UNDEFINED();
    }

    return Invoker<R, P1, P2, P3, P4>::Go(args, holder->callback, a1, a2, a3,
        a4);
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4,
    typename P5>
struct Dispatcher<R(P1, P2, P3, P4, P5)> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R(P1, P2, P3, P4, P5)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    typename CallbackParamTraits<P1>::LocalType a1;
    typename CallbackParamTraits<P2>::LocalType a2;
    typename CallbackParamTraits<P3>::LocalType a3;
    typename CallbackParamTraits<P4>::LocalType a4;
    typename CallbackParamTraits<P5>::LocalType a5;
    if (!GetNextArgument(&args, holder->flags, true, &a1) ||
        !GetNextArgument(&args, holder->flags, false, &a2) ||
        !GetNextArgument(&args, holder->flags, false, &a3) ||
        !GetNextArgument(&args, holder->flags, false, &a4) ||
        !GetNextArgument(&args, holder->flags, false, &a5)) {
      args.ThrowError();
      MATE_METHOD_RETURN_UNDEFINED();
    }

    return Invoker<R, P1, P2, P3, P4, P5>::Go(args, holder->callback, a1, a2,
        a3, a4, a5);
  }
};

template<typename R, typename P1, typename P2, typename P3, typename P4,
    typename P5, typename P6>
struct Dispatcher<R(P1, P2, P3, P4, P5, P6)> {
  static MATE_METHOD(DispatchToCallback) {
    Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());

    typedef CallbackHolder<R(P1, P2, P3, P4, P5, P6)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);

    typename CallbackParamTraits<P1>::LocalType a1;
    typename CallbackParamTraits<P2>::LocalType a2;
    typename CallbackParamTraits<P3>::LocalType a3;
    typename CallbackParamTraits<P4>::LocalType a4;
    typename CallbackParamTraits<P5>::LocalType a5;
    typename CallbackParamTraits<P6>::LocalType a6;
    if (!GetNextArgument(&args, holder->flags, true, &a1) ||
        !GetNextArgument(&args, holder->flags, false, &a2) ||
        !GetNextArgument(&args, holder->flags, false, &a3) ||
        !GetNextArgument(&args, holder->flags, false, &a4) ||
        !GetNextArgument(&args, holder->flags, false, &a5) ||
        !GetNextArgument(&args, holder->flags, false, &a6)) {
      args.ThrowError();
      MATE_METHOD_RETURN_UNDEFINED();
    }

    return Invoker<R, P1, P2, P3, P4, P5, P6>::Go(args, holder->callback, a1,
        a2, a3, a4, a5, a6);
  }
};

}  // namespace internal


// CreateFunctionTemplate creates a v8::FunctionTemplate that will create
// JavaScript functions that execute a provided C++ function or base::Callback.
// JavaScript arguments are automatically converted via mate::Converter, as is
// the return value of the C++ function, if any.
template<typename Sig>
v8::Local<v8::FunctionTemplate> CreateFunctionTemplate(
    v8::Isolate* isolate, const base::Callback<Sig> callback,
    int callback_flags = 0) {
  typedef internal::CallbackHolder<Sig> HolderT;
  HolderT* holder = new HolderT(isolate, callback, callback_flags);

  return v8::FunctionTemplate::New(
#if NODE_VERSION_AT_LEAST(0, 11, 11)
      isolate,
#endif
      &internal::Dispatcher<Sig>::DispatchToCallback,
      ConvertToV8<v8::Handle<v8::External> >(isolate,
                                             holder->GetHandle(isolate)));
}

}  // namespace mate

#endif  // NATIVE_MATE_FUNCTION_TEMPLATE_H_