/*
 *  SourceManager.hh - Class that handles multiple sessions dynamically.
 *  Copyright (C) 2014  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of liveMediaStreamer.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  David Cassany <david.cassany@i2cat.net>,
 *            
 */
#ifndef _SOURCE_MANAGER_HH
#define _SOURCE_MANAGER_HH

#include "../../Filter.hh"
#include "Handlers.hh"

#include <thread>
#include <map>
#include <list>
#include <functional>
#include <string>
#include <liveMedia/liveMedia.hh>
#include <BasicUsageEnvironment.hh>

#define ID_LENGTH 4
#define PROTOCOL "RTP"
 

class Session;

class StreamClientState {
public:
    StreamClientState(std::string id_);
    virtual ~StreamClientState();
    
    std::string getId(){return id;};

public:
    MediaSubsessionIterator* iter;
    MediaSession* session;
    MediaSubsession* subsession;
    TaskToken streamTimerTask;
    double duration;
    
private:
    std::string id;
};

class SourceManager : public HeadFilter {
private:
    SourceManager(int writersNum = MAX_WRITERS);
    
public:
    static SourceManager* getInstance();
    static void destroyInstance();
    
    static std::string makeSessionSDP(std::string sessionName, std::string sessionDescription);
    static std::string makeSubsessionSDP(std::string mediumName, std::string protocolName, 
                                  unsigned int RTPPayloadFormat, 
                                  std::string codecName, unsigned int bandwidth, 
                                  unsigned int RTPTimestampFrequency, 
                                  unsigned int clientPortNum = 0,
                                  unsigned int channels = 0);
      
    bool runManager();
    bool isRunning();
    
    void closeManager();

    bool addSession(Session* session);
    bool removeSession(std::string id);
    
    Session* getSession(std::string id);
    int getWriterID(unsigned int port);
    void setCallback(std::function<void(char const*, unsigned short)> callbackFunction);
    
    UsageEnvironment* envir() {return env;};
    
protected:
    void initializeEventMap();
    
private:
    friend bool handlers::addSubsessionSink(UsageEnvironment& env, MediaSubsession *subsession);
    void doGetState(Jzon::Object &filterNode) {/*TODO*/};
    void addSessionEvent(Jzon::Node* params, Jzon::Object &outputNode);

    void addConnection(int wId, MediaSubsession* subsession);
    
    static void* startServer(void *args);
    FrameQueue *allocQueue(int wId);
      
    std::thread mngrTh;
    
    static SourceManager* mngrInstance;
    std::map<std::string, Session*> sessionMap;
    UsageEnvironment* env;
    uint8_t watch;
    std::function<void(char const*, unsigned short)> callback;
    
};

class Session {
public:
    static Session* createNewByURL(UsageEnvironment& env, std::string progName, std::string rtspURL, std::string id);
    static Session* createNew(UsageEnvironment& env, std::string sdp, std::string id);
    
    virtual ~Session();
    
    std::string getId() {return scs->getId();};
    MediaSubsession* getSubsessionByPort(int port);
    
    bool initiateSession();
    
protected:
    Session(std::string id);
    
    RTSPClient* client;
    StreamClientState *scs;
};

#endif