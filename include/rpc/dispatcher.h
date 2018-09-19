#pragma once

#ifndef DISPATCHER_H_CXIVZD5L
#define DISPATCHER_H_CXIVZD5L

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>

#include "rpc/config.h"
#include "rpc/msgpack/msgpack.hpp"

#include "rpc/detail/call.h"
#include "rpc/detail/func_tools.h"
#include "rpc/detail/func_traits.h"
#include "rpc/detail/log.h"
#include "rpc/detail/not.h"
#include "rpc/detail/response.h"
#include "rpc/detail/make_unique.h"

namespace rpc {

namespace detail {
	class EventManager
	{//负责保存函数对象，负责绑定，与dispatcher解耦
		public:
			EventManager(){};

			virtual EventManager& operator=(EventManager& a)
			{
				funcs_=a.funcs_;
				return *this;
			}

			//! \brief This functor type unifies the interfaces of functions that are
			//!        called remotely
			using adaptor_type = std::function<std::unique_ptr<RPCLIB_MSGPACK::object_handle>(
					RPCLIB_MSGPACK::object const &)>;

			std::unordered_map<std::string, adaptor_type> funcs_;

			//! \brief Checks the argument count and throws an exception if
			//! it is not the expected amount.
			static void enforce_arg_count(std::string const &func, std::size_t found,
					std::size_t expected);

			void enforce_unique_name(std::string const &func);
			//! \brief Binds a functor to a name so it becomes callable via RPC.
			//! \param name The name of the functor.
			//! \param func The functor to bind.
			//! \tparam F The type of the functor.
			template <typename F> void bind(std::string const &name, F func);

			//! \defgroup Tag-dispatched bind implementations for various functor cases.

			//! \brief Stores a void, zero-arg functor with a name.
			template <typename F>
				void bind(std::string const &name, F func,
						detail::tags::void_result const &,
						detail::tags::zero_arg const &);

			//! \brief Stores a void, non-zero-arg functor with a name.
			template <typename F>
				void bind(std::string const &name, F func,
						detail::tags::void_result const &,
						detail::tags::nonzero_arg const &);

			//! \brief Stores a non-void, zero-arg functor with a name.
			template <typename F>
				void bind(std::string const &name, F func,
						detail::tags::nonvoid_result const &,
						detail::tags::zero_arg const &);

			//! \brief Stores a non-void, non-zero-arg functor with a name.
			template <typename F>
				void bind(std::string const &name, F func,
						detail::tags::nonvoid_result const &,
						detail::tags::nonzero_arg const &);

			template <typename T> RPCLIB_MSGPACK::object pack(T &&arg);
	};


//! \brief This class maintains a registry of functors associated with their
//! names, and callable using a msgpack-rpc call pack.
//dispatcher 抽象类，不包含bind,通过EventManager进行函数绑定管理
class default_dispatcher
{
public:
    
    EventManager event_handle;
    //! \brief This is the type of messages as per the msgpack-rpc spec.
    using call_t = std::tuple<int8_t, uint32_t, std::string, RPCLIB_MSGPACK::object>;

    //! \brief This is the type of notification messages.
    using notification_t = std::tuple<int8_t, std::string, RPCLIB_MSGPACK::object>;


    //! \brief Processes a message that contains a call according to
    //! the Msgpack-RPC spec.
    //! \param msg The buffer that contains the messagepack.
    //! \throws std::runtime_error If the messagepack does not contain a
    //! a call or the types of the parameters are not convertible to the called
    //! functions' parameters.
    virtual void dispatch(RPCLIB_MSGPACK::sbuffer const &msg);

    //! \brief Processes a message that contains a call according to
    //! the Msgpack-RPC spec.
    //! \param msg The messagepack object that contains the call.
    //! \param suppress_exceptions If true, exceptions will be caught and
    //! written
    //! as response for the client.
    //! \throws std::runtime_error If the types of the parameters are not
    //! convertible to the called functions' parameters.
    virtual detail::response dispatch(RPCLIB_MSGPACK::object const &msg,
                              bool suppress_exceptions = false);

    enum class request_type { notification=2, call=1 };
    
    //template <typename T> RPCLIB_MSGPACK::object pack(T &&arg);

private:

	//! \brief Dispatches a call (which will have a response).
    virtual detail::response dispatch_call(RPCLIB_MSGPACK::object const &msg,
                                   bool suppress_exceptions = false);
	

    //! \brief Dispatches a notification (which will not have a response)
    virtual detail::response dispatch_notification(RPCLIB_MSGPACK::object const &msg,
                                           bool suppress_exceptions = false);


    RPCLIB_CREATE_LOG_CHANNEL(default_dispatcher)
};
}//detail
}//rpc

#include "dispatcher.inl"

#endif /* end of include guard: DISPATCHER_H_CXIVZD5L */
