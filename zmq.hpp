/*
    Copyright (c) 2009-2011 250bpm s.r.o.
    Copyright (c) 2011 Botond Ballo
    Copyright (c) 2007-2009 iMatix Corporation

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef __ZMQ_HPP_INCLUDED__
#define __ZMQ_HPP_INCLUDED__

#include <zmq.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>

//  Detect whether the compiler supports C++11 rvalue references.
#if (defined(__GNUC__) && (__GNUC__ > 4 || \
      (__GNUC__ == 4 && __GNUC_MINOR__ > 2)) && \
      defined(__GXX_EXPERIMENTAL_CXX0X__))
    #define ZMQ_HAS_RVALUE_REFS
    #define ZMQ_DELETED_FUNCTION = delete
#elif defined(__clang__)
    #if __has_feature(cxx_rvalue_references)
        #define ZMQ_HAS_RVALUE_REFS
    #endif

    #if __has_feature(cxx_deleted_functions)
        #define ZMQ_DELETED_FUNCTION = delete
    #else
        #define ZMQ_DELETED_FUNCTION
    #endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1600)
    #define ZMQ_HAS_RVALUE_REFS
    #define ZMQ_DELETED_FUNCTION
#else
    #define ZMQ_DELETED_FUNCTION
#endif

// In order to prevent unused variable warnings when building in non-debug
// mode use this macro to make assertions.
#ifndef NDEBUG
#   define ZMQ_ASSERT(expression) assert(expression)
#else
#   define ZMQ_ASSERT(expression) (expression)
#endif


/*
 * TODO SRombauts : Doygen documentation !
 * TODO SRombauts : dump() message in ascii and hexadecimal
 * TODO SRombauts : message_t() ctor zerocopy with a provided free_fn (using delete)
 * TODO SRombauts : use all the following functions (instead of deprecated ones)
 * TODO SRombauts : switch at compile time between 2.2 and 3.2 functions
 * TODO SRombauts : encapsulation of zmq_proxy without void* : proxy(socket&, socket&, socket&)
 * TODO SRombauts : real encapsulation of zmq_poll
 * TODO SRombauts : encapsulation for multi-part messages in a new class
 * TODO SRombauts : provide some C++ style, like streams or type conversion
 * TODO SRombauts : see what high level C binding do (czmq)
 * TODO SRombauts : see what other C++ binding do (zmqpp and zmqmessage)
 * TODO SRombauts : redefine in the namespace all constants from zmq.h, for easy access ?
 * TODO SRombauts : add general documentation in the README and here
 
zmq_ctx_destroy - destroy a 0MQ context
zmq_ctx_get - get context options
zmq_ctx_new - create new 0MQ context
zmq_ctx_set - set context options

zmq_disconnect - Disconnect a socket

zmq_msg_get - get message property
zmq_msg_more - indicate if there are more message parts to receive
zmq_msg_set - set message property
zmq_poll - input/output multiplexing
zmq_proxy - start built-in 0MQ proxy
zmq_socket_monitor - register a monitoring callback
zmq_unbind - Stop accepting connections on a socket
*/


namespace zmq
{
    /******************************************************************************/
    /*  0MQ socket definition.                                                    */
    /******************************************************************************/

    /*  Socket types.                                                             */
    enum socket_types
    {
        PAIR = ZMQ_PAIR, // 0
        PUB = ZMQ_PUB,  // 1
        SUB = ZMQ_SUB,  // 2
        REQ = ZMQ_REQ,  // 3
        REP = ZMQ_REP,  // 4
        DEALER = ZMQ_DEALER, // 5
        ROUTER = ZMQ_ROUTER, // 6
        PULL = ZMQ_PULL, // 7
        PUSH = ZMQ_PUSH, // 8
        XPUB = ZMQ_XPUB, // 9
        XSUB = ZMQ_XSUB  // 10
    };

    /*  Socket options.                                                           */
    enum socket_options
    {
        AFFINITY = ZMQ_AFFINITY, // 4
        IDENTITY = ZMQ_IDENTITY, // 5
        SUBSCRIBE = ZMQ_SUBSCRIBE, // 6
        UNSUBSCRIBE = ZMQ_UNSUBSCRIBE, // 7
        RATE = ZMQ_RATE, // 8
        RECOVERY_IVL = ZMQ_RECOVERY_IVL, // 9
        SNDBUF = ZMQ_SNDBUF, // 11
        RCVBUF = ZMQ_RCVBUF, // 12
        RCVMORE = ZMQ_RCVMORE, // 13
        FD = ZMQ_FD, // 14
        EVENTS = ZMQ_EVENTS, // 15
        TYPE = ZMQ_TYPE, // 16
        LINGER = ZMQ_LINGER, // 17
        RECONNECT_IVL = ZMQ_RECONNECT_IVL, // 18
        BACKLOG = ZMQ_BACKLOG, // 19
        RECONNECT_IVL_MAX = ZMQ_RECONNECT_IVL_MAX, // 21
        MAXMSGSIZE = ZMQ_MAXMSGSIZE, // 22
        SNDHWM = ZMQ_SNDHWM, // 23
        RCVHWM = ZMQ_RCVHWM, // 24
        MULTICAST_HOPS = ZMQ_MULTICAST_HOPS, // 25
        RCVTIMEO = ZMQ_RCVTIMEO, // 27
        SNDTIMEO = ZMQ_SNDTIMEO, // 28
        IPV4ONLY = ZMQ_IPV4ONLY, // 31              /*  Request replacement by IPV6          */
        LAST_ENDPOINT = ZMQ_LAST_ENDPOINT, // 32
        ROUTER_MANDATORY = ZMQ_ROUTER_MANDATORY, // 33
        TCP_KEEPALIVE = ZMQ_TCP_KEEPALIVE, // 34
        TCP_KEEPALIVE_CNT = ZMQ_TCP_KEEPALIVE_CNT, // 35
        TCP_KEEPALIVE_IDLE = ZMQ_TCP_KEEPALIVE_IDLE, // 36
        TCP_KEEPALIVE_INTVL = ZMQ_TCP_KEEPALIVE_INTVL, // 37
        TCP_ACCEPT_FILTER = ZMQ_TCP_ACCEPT_FILTER, // 38
        DELAY_ATTACH_ON_CONNECT = ZMQ_DELAY_ATTACH_ON_CONNECT, // 39
        XPUB_VERBOSE = ZMQ_XPUB_VERBOSE, // 40
        ROUTER_RAW = ZMQ_ROUTER_RAW, // 41
        IPV6 = ZMQ_IPV6 // 42
    };

    /*  Message options                                                           */
    enum message_options
    {
        MORE = ZMQ_MORE // 1
    };

    /*  Send/recv options.                                                        */
    enum send_recv_options
    {
        DONTWAIT = ZMQ_DONTWAIT, // 1
        SNDMORE = ZMQ_SNDMORE // 2
    };

    typedef zmq_free_fn free_fn;
    typedef zmq_pollitem_t pollitem_t;

    class error_t : public std::exception
    {
    public:

        error_t () : errnum (zmq_errno ()) {}

        virtual const char *what () const throw ()
        {
            return zmq_strerror (errnum);
        }

        int num () const
        {
            return errnum;
        }

    private:

        int errnum;
    };

    inline int poll (zmq_pollitem_t *items_, int nitems_, long timeout_ = -1)
    {
        int rc = zmq_poll (items_, nitems_, timeout_);
        if (rc < 0)
            throw error_t ();
        return rc;
    }

    inline void proxy (void *frontend, void *backend, void *capture)
    {
        int rc = zmq_proxy (frontend, backend, capture);
        if (rc != 0)
            throw error_t ();
    }

    inline void version (int *major_, int *minor_, int *patch_)
    {
        zmq_version (major_, minor_, patch_);
    }

    class message_t
    {
        friend class socket_t;

    public:

        inline message_t ()
        {
            int rc = zmq_msg_init (&msg);
            if (rc != 0)
                throw error_t ();
        }

        inline explicit message_t (size_t size_)
        {
            int rc = zmq_msg_init_size (&msg, size_);
            if (rc != 0)
                throw error_t ();
        }

        inline explicit message_t (void *data_, size_t size_)
        {
            int rc = zmq_msg_init_size (&msg, size_);
            if (rc != 0)
                throw error_t ();
            memcpy (data(), data_, size_);
        }

        inline message_t (void *data_, size_t size_, free_fn *ffn_,
            void *hint_ = NULL)
        {
            int rc = zmq_msg_init_data (&msg, data_, size_, ffn_, hint_);
            if (rc != 0)
                throw error_t ();
        }

#ifdef ZMQ_HAS_RVALUE_REFS
        inline message_t (message_t &&rhs) : msg (rhs.msg)
        {
            int rc = zmq_msg_init (&rhs.msg);
            if (rc != 0)
                throw error_t ();
        }

        inline message_t &operator = (message_t &&rhs)
        {
            std::swap (msg, rhs.msg);
            return *this;
        }
#endif

        inline ~message_t ()
        {
            int rc = zmq_msg_close (&msg);
            ZMQ_ASSERT (rc == 0);
        }

        inline void rebuild ()
        {
            int rc = zmq_msg_close (&msg);
            if (rc != 0)
                throw error_t ();
            rc = zmq_msg_init (&msg);
            if (rc != 0)
                throw error_t ();
        }

        inline void rebuild (size_t size_)
        {
            int rc = zmq_msg_close (&msg);
            if (rc != 0)
                throw error_t ();
            rc = zmq_msg_init_size (&msg, size_);
            if (rc != 0)
                throw error_t ();
        }

        inline void rebuild (void *data_, size_t size_, free_fn *ffn_,
            void *hint_ = NULL)
        {
            int rc = zmq_msg_close (&msg);
            if (rc != 0)
                throw error_t ();
            rc = zmq_msg_init_data (&msg, data_, size_, ffn_, hint_);
            if (rc != 0)
                throw error_t ();
        }

        inline void move (message_t *msg_)
        {
            int rc = zmq_msg_move (&msg, &(msg_->msg));
            if (rc != 0)
                throw error_t ();
        }

        inline void copy (message_t *msg_)
        {
            int rc = zmq_msg_copy (&msg, &(msg_->msg));
            if (rc != 0)
                throw error_t ();
        }

        inline void *data ()
        {
            return zmq_msg_data (&msg);
        }

        inline const void* data () const
        {
            return zmq_msg_data (const_cast<zmq_msg_t*>(&msg));
        }

        inline size_t size () const
        {
            return zmq_msg_size (const_cast<zmq_msg_t*>(&msg));
        }

        inline std::string string() const
        {
            std::string string;
            return string.assign((const char*)data(), size());
        }

    private:

        //  The underlying message
        zmq_msg_t msg;

        //  Disable implicit message copying, so that users won't use shared
        //  messages (less efficient) without being aware of the fact.
        message_t (const message_t&);
        void operator = (const message_t&);
    };

    class context_t
    {
        friend class socket_t;

    public:

        inline explicit context_t (int io_threads_)
        {
            ptr = zmq_init (io_threads_);
            if (ptr == NULL)
                throw error_t ();
        }

#ifdef ZMQ_HAS_RVALUE_REFS
        inline context_t (context_t &&rhs) : ptr (rhs.ptr)
        {
            rhs.ptr = NULL;
        }
        inline context_t &operator = (context_t &&rhs)
        {
            std::swap (ptr, rhs.ptr);
            return *this;
        }
#endif

        inline ~context_t ()
        {
            close();
        }

        inline void close()
        {
            if (ptr == NULL)
                return;
            int rc = zmq_term (ptr);
            ZMQ_ASSERT (rc == 0);
            ptr = NULL;
        }

        //  Be careful with this, it's probably only useful for
        //  using the C api together with an existing C++ api.
        //  Normally you should never need to use this.
        inline operator void* ()
        {
            return ptr;
        }

    private:

        void *ptr;

        context_t (const context_t&);
        void operator = (const context_t&);
    };

    class socket_t
    {
    public:

        inline socket_t (context_t &context_, int type_)
        {
            ptr = zmq_socket (context_.ptr, type_);
            if (ptr == NULL)
                throw error_t ();
        }

#ifdef ZMQ_HAS_RVALUE_REFS
        inline socket_t(socket_t&& rhs) : ptr(rhs.ptr)
        {
            rhs.ptr = NULL;
        }
        inline socket_t& operator=(socket_t&& rhs)
        {
            std::swap(ptr, rhs.ptr);
            return *this;
        }
#endif

        inline ~socket_t ()
        {
            close();
        }

        //  Be careful with this, it's probably only useful for
        //  using the C api together with an existing C++ api.
        //  Normally you should never need to use this.
        inline operator void* ()
        {
            return ptr;
        }

        inline void close()
        {
            if(ptr == NULL)
                // already closed
                return ;
            int rc = zmq_close (ptr);
            ZMQ_ASSERT (rc == 0);
            ptr = 0 ;
        }

        inline void setsockopt (int option_, const void *optval_,
            size_t optvallen_)
        {
            int rc = zmq_setsockopt (ptr, option_, optval_, optvallen_);
            if (rc != 0)
                throw error_t ();
        }

        inline void getsockopt (int option_, void *optval_,
            size_t *optvallen_)
        {
            int rc = zmq_getsockopt (ptr, option_, optval_, optvallen_);
            if (rc != 0)
                throw error_t ();
        }

        inline void bind (const char *addr_)
        {
            int rc = zmq_bind (ptr, addr_);
            if (rc != 0)
                throw error_t ();
        }

        inline void connect (const char *addr_)
        {
            int rc = zmq_connect (ptr, addr_);
            if (rc != 0)
                throw error_t ();
        }

        inline bool connected()
        {
            return(ptr != NULL);
        }

        inline size_t send (const void *buf_, size_t len_, int flags_ = 0)
        {
            int nbytes = zmq_send (ptr, buf_, len_, flags_);
            if (nbytes >= 0)
                return (size_t) nbytes;
            if (zmq_errno () == EAGAIN)
                return 0;
            throw error_t ();
        }

        inline bool send (message_t &msg_, int flags_ = 0)
        {
            int nbytes = zmq_msg_send (&(msg_.msg), ptr, flags_);
            if (nbytes >= 0)
                return true;
            if (zmq_errno () == EAGAIN)
                return false;
            throw error_t ();
        }

        inline size_t recv (void *buf_, size_t len_, int flags_ = 0)
        {
            int nbytes = zmq_recv (ptr, buf_, len_, flags_);
            if (nbytes >= 0)
                return (size_t) nbytes;
            if (zmq_errno () == EAGAIN)
                return 0;
            throw error_t ();
        }

        inline bool recv (message_t& msg_, int flags_ = 0)
        {
            int nbytes = zmq_msg_recv (&(msg_.msg), ptr, flags_);
            if (nbytes >= 0)
                return true;
            if (zmq_errno () == EAGAIN)
                return false;
            throw error_t ();
        }

    private:

        void *ptr;

        socket_t (const socket_t&) ZMQ_DELETED_FUNCTION;
        void operator = (const socket_t&) ZMQ_DELETED_FUNCTION;
    };

}

#endif
