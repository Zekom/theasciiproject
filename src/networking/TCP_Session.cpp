//
//  TCP_Session.cpp
//  The ASCII Project
//
//  Created by Jonathan Rumion on 3/1/13.
//  Copyright (c) 2013 TAP. All rights reserved.
//

#include "Packets.h"
#include "TCP_Session.h"
#include "TCP_Participant.h"
#include "../api/ServerAPI.h"
#include "../utils/FileLogger.h"
#include <boost/bind.hpp>
#include <memory>
#include <iostream>

extern FileLogger *fileLogger;

void TCP_Session::start(){
    
    fileLogger->ErrorLog("Client " + getClientIP() + ": Connected");
    // Disable Nagles Algorithm - http://en.wikipedia.org/wiki/Nagle's_algorithm
    tcp_socket.set_option(boost::asio::ip::tcp::no_delay(true));
    // Join our tcp client pool
    tcp_pool.join(shared_from_this());
    
    kickStart();
}


/*
 
 Wrap all of our client cleanup into one function.
 
 */
void TCP_Session::end(){
    
    tcp_pool.leave(shared_from_this());
    fileLogger->ErrorLog("Client " + clientIP + ": Disconnected");
    
}



// Executed at TCP_Session::start()
std::string TCP_Session::getClientIP(){
    
    boost::asio::ip::tcp::endpoint remote_ep = tcp_socket.remote_endpoint();
    boost::asio::ip::address remote_ad = remote_ep.address();
    std::string remoteIP = remote_ad.to_string();
    
    clientIP = remoteIP;
    return remoteIP;
    
}


// Start the async cycle
void TCP_Session::kickStart(){
    
    boost::asio::async_write(tcp_socket, boost::asio::buffer(std::string("Welcome to The ASCII Project\n\n"
                                                                         "Commands Available: \n"
                                                                         "------------------- \n\n"
                                                                         "term \n"
                                                                         "raw \n"
                                                                         "quit \r\n\r\n")),
                             boost::bind(&TCP_Session::startSession, shared_from_this(), boost::asio::placeholders::error ));
    
}


// Parse our Input
void TCP_Session::startSession(const boost::system::error_code& error)
{
    if (!error)
    {
        boost::asio::async_read_until(tcp_socket, raw_line_command, "\r\n", boost::bind(&TCP_Session::end, shared_from_this() ) );
    }
    else
    {
        end();
    }
}