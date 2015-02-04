/*
 ****************************************************************************
 *  Copyright (c) 2014 Uriah Liggett <hairu526@gmail.com>                   *
 *	This file is part of FreeLSS.                                           *
 *                                                                          *
 *  FreeLSS is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, either version 3 of the License, or       *
 *  (at your option) any later version.                                     *
 *                                                                          *
 *  FreeLSS is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with FreeLSS.  If not, see <http://www.gnu.org/licenses/>.       *
 ****************************************************************************
*/

#include "Main.h"
#include "Thread.h"

namespace scanner
{

// The threaded function
static void * G_Thread_ThreadFunc ( void *ptr )
{
	Thread * thread = reinterpret_cast<Thread *>(ptr);

	try
	{
		thread->run();
	}
	catch (scanner::Exception& ex)
	{
		std::cerr << "Exception thrown: " << ex << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unknown Exception Thrown" << std::endl;
	}
	
	return NULL;
}

Thread::Thread() : 	
	m_stopRequested(false),
	m_handle()	
{
	// Do nothing
}

Thread::~Thread()
{
	// Do nothing
}

void Thread::execute()				/** Begins execution of the thread */
{
	m_stopRequested = false;

	if (pthread_create(&m_handle, NULL, G_Thread_ThreadFunc, (void*) this) != 0)
	{
		throw Exception("Error creating thread");
	}
}

void Thread::join()					/** Blocks the active thread until this thread ends */
{
	if (pthread_join(m_handle, NULL) != 0)
	{
		throw Exception("Error joining thread");
	}
}

void Thread::stop()				/** Indicates to the thread that we want it to stop but does not forcefully kill it */
{
	m_stopRequested = true;
}

void Thread::usleep(unsigned long microseconds)		/** Sleeps the active thread the specified number of MICROSECONDS */
{
	if (::usleep(microseconds) != 0)
	{
		std::stringstream sstr;
		sstr << "Error sleeping thread, errno=" << errno;
		std::cerr << sstr.str() << std::endl;
	}
}

}
