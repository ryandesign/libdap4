
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 
// (c) COPYRIGHT URI/MIT 1994-2002
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      jhrg,jimg       James Gallagher <jgallagher@gso.uri.edu>

#include "config_dap.h"

static char rcsid[] not_used =
    { "$Id: SignalHandler.cc,v 1.3 2004/02/19 19:42:52 jimg Exp $" };

#include <signal.h>

#ifndef WIN32
#include <unistd.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include "SignalHandler.h"
#include "util.h"

EventHandler *SignalHandler::d_signal_handlers[NSIG];
Sigfunc *SignalHandler::d_old_handlers[NSIG];
SignalHandler *SignalHandler::d_instance = 0;

// instance_control is used to ensure that in a MT environment d_instance is
// correctly initialized.
#if HAVE_PTHREAD_H
static pthread_once_t instance_control = PTHREAD_ONCE_INIT;
#endif

/// Private static void method.
void
SignalHandler::initialize_instance()
{
    // MT-Safe if called via pthread_once or similar
    SignalHandler::d_instance = new SignalHandler;
    atexit(SignalHandler::delete_instance);
}

/// Private static void method.
void 
SignalHandler::delete_instance()
{
    if (SignalHandler::d_instance) {
#if HAVE_PTHREAD_H
	instance_control = PTHREAD_ONCE_INIT;
#endif
	for (int i = 0; i < NSIG; ++i) {
	    d_signal_handlers[i] = 0;
	    d_old_handlers[i] = 0;
	}

	delete SignalHandler::d_instance; 
	SignalHandler::d_instance = 0;
    }
}

/** This private method is the adapter between the C-style interface of the
    signal sub-system and C++'s method interface. This uses the lookup table
    to find an instance of EventHandler and calls that instance's
    handle_signal method.

    @param signum The number of the signal. */
void 
SignalHandler::dispatcher(int signum)
{
    // Perform a sanity check...
    if (SignalHandler::d_signal_handlers[signum] != 0)
	// Dispatch the handler's hook method.
	SignalHandler::d_signal_handlers[signum]->handle_signal(signum);
    
    Sigfunc *old_handler = SignalHandler::d_old_handlers[signum];
    if (old_handler == SIG_IGN || old_handler == SIG_ERR)
	return;
    else if (old_handler == SIG_DFL) {
	switch (signum) {
	  case SIGHUP:
	  case SIGINT:
	  case SIGKILL:
	  case SIGPIPE:
	  case SIGALRM:
	  case SIGTERM:
	  case SIGUSR1:
	  case SIGUSR2: _exit(EXIT_FAILURE);

	    // register_handler() should never allow any fiddling with
	    // signals other than those listed above.
	  default: abort();
	}
    }
    else
	old_handler(signum);
}

/** Get a pointer to the single instance of SignalHandler. */
SignalHandler* 
SignalHandler::instance()
{
#if HAVE_PTHREAD_H
    pthread_once(&instance_control, initialize_instance);
#else
    if (!d_instance)
	initialize_signal_handler();
#endif

    return d_instance;
}

/** Register an event handler. By default run any previously registered
    action/handler such as those installed using \c sigaction(). For signals
    such as SIGALRM (the alarm signal) this may not be what you want; see the
    \e override parameter. See also the class description.

    @param signum Bind the event handler to this signal number. Limited to
    those signals that, according to POSIX.1, cause process termination.
    @param eh A pointer to the EventHandler for \c signum.
    @param override If \c true, do not run the default handler/action.
    Instead run \e eh and then treat the signal as if the original action was
    SIG_IGN. Default is false.
    @return A pointer to the old EventHandler or null. */
EventHandler *
SignalHandler::register_handler(int signum, EventHandler *eh, bool override) 
    throw(InternalErr)
{
    // Check first for improper use.
    switch (signum) {
      case SIGHUP:
      case SIGINT:
      case SIGKILL:
      case SIGPIPE:
      case SIGALRM:
      case SIGTERM:
      case SIGUSR1:
      case SIGUSR2: break;

      default: throw InternalErr(__FILE__, __LINE__,
		  string("Call to register_handler with unsupported signal (")
		  + long_to_string(signum) + string(")."));
    }

    // Save the old EventHandler
    EventHandler *old_eh = SignalHandler::d_signal_handlers[signum];

    SignalHandler::d_signal_handlers[signum] = eh;
 
    // Register the dispatcher to handle this signal. See Stevens, Advanced
    // Programming in the UNIX Environment, p.298.
    struct sigaction sa;
    sa.sa_handler = dispatcher;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    // Try to suppress restarting system calls if we're handling an alarm.
    // This lets alarms block I/O calls that would normally restart. 07/18/03
    // jhrg
    if (signum == SIGALRM) {
#ifdef SA_INTERUPT
	sa.sa_flags |= SA_INTERUPT;
#endif
    }
    else {
#ifdef SA_RESTART
	sa.sa_flags |= SA_RESTART;
#endif
    }

    struct sigaction osa;	// extract the old handler/action

    if (sigaction(signum, &sa, &osa) < 0)
	throw InternalErr(__FILE__, __LINE__, "Could not register a signal handler.");

    // Take care of the case where this interface is used to register a
    // handler more than once. We want to make sure that the dispatcher is
    // not installed as the 'old handler' because that results in an infinite
    // loop. 02/10/04 jhrg
    if (override)
	SignalHandler::d_old_handlers[signum] = SIG_IGN;
    else if (osa.sa_handler != dispatcher)
	SignalHandler::d_old_handlers[signum] = osa.sa_handler;

    return old_eh;
}

/** Remove the event hander.
    @param signum The signal number of the handler to remove. 
    @return The old event handler */
EventHandler *
SignalHandler::remove_handler(int signum)
{
    EventHandler *old_eh = SignalHandler::d_signal_handlers[signum];

    SignalHandler::d_signal_handlers[signum] = 0;

    return old_eh;
}

// $Log: SignalHandler.cc,v $
// Revision 1.3  2004/02/19 19:42:52  jimg
// Merged with release-3-4-2FCS and resolved conflicts.
//
// Revision 1.1.2.5  2004/02/11 17:12:01  jimg
// Changed how this class creates its instance. It now does the same thing as
// RCReader and is much more in line with HTTPCache.
//
// Revision 1.1.2.4  2004/02/10 20:48:13  jimg
// Added support for running old handlers/actions once the newly registered
// handlers are executed. Also limited the signals to those that POSIX.1 defines
// as terminating the process by default. The code won't try to work with other
// signals. It's possible to get it to do so, but fairly involved. See Steven's
// "Advanced Programming ..." book for the lowdown on signals.
//
// Revision 1.2  2003/12/08 18:02:29  edavis
// Merge release-3-4 into trunk
//
// Revision 1.1.2.3  2003/12/05 15:59:52  dan
// Fixed compile error in #if HAVE_PTHREAD_H, else clause.  Was using
// '_instance' when it should be 'd_instance'.
//
// Revision 1.1.2.2  2003/07/19 01:48:02  jimg
// Fixed up some comments.
//
// Revision 1.1.2.1  2003/07/19 01:47:43  jimg
// Added.
//