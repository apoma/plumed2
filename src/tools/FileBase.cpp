/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2013 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed-code.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "File.h"
#include "Exception.h"
#include "core/Action.h"
#include "core/PlumedMain.h"
#include "core/Value.h"
#include "Communicator.h"
#include "Tools.h"
#include <cstdarg>
#include <cstring>

#include <iostream>
#include <string>

namespace PLMD{

void FileBase::test(){
  PLMD::OFile pof;
  pof.open("ciao");
  pof.printf("%s\n","test1");
  pof.setLinePrefix("plumed: ");
  pof.printf("%s\n","test2");
  pof.setLinePrefix("");
  pof.addConstantField("x2").printField("x2",67.0);
  pof.printField("x1",10.0).printField("x3",20.12345678901234567890).printField();
  pof.printField("x1",10.0).printField("x3",-1e70*20.12345678901234567890).printField();
  pof.printField("x3",10.0).printField("x2",777.0).printField("x1",-1e70*20.12345678901234567890).printField();
  pof.printField("x3",67.0).printField("x1",18.0).printField();
  pof.close();

  PLMD::IFile pif;
  std::string s;
  pif.open("ciao");
  pif.getline(s); std::printf("%s\n",s.c_str());
  pif.getline(s); std::printf("%s\n",s.c_str());
  
  int x1,x2,x3;
  while(pif.scanField("x1",x1).scanField("x3",x2).scanField("x2",x3).scanField()){
    std::cout<<"CHECK "<<x1<<" "<<x2<<" "<<x3<<"\n";
  }
  pif.close();
}

FileBase& FileBase::link(FILE*fp){
  this->fp=fp;
  cloned=true;
  return *this;
}

FileBase& FileBase::flush(){
  fflush(fp);
  if(heavyFlush){
    fclose(fp);
    fp=std::fopen(const_cast<char*>(path.c_str()),"a");
  }
  return *this;
}

FileBase& FileBase::link(Communicator&comm){
  plumed_massert(!fp,"cannot link an already open file");
  this->comm=&comm;
  return *this;
}

FileBase& FileBase::link(PlumedMain&plumed){
  plumed_massert(!fp,"cannot link an already open file");
  this->plumed=&plumed;
  link(plumed.comm);
  return *this;
}

FileBase& FileBase::link(Action&action){
  plumed_massert(!fp,"cannot link an already open file");
  this->action=&action;
  link(action.plumed);
  return *this;
}

FileBase& FileBase::open(const std::string& path,const std::string& mode){
  plumed_massert(!cloned,"file "+path+" appears to be cloned");
  eof=false;
  err=false;
  fp=NULL;
  if(plumed){
    this->path=path+plumed->getSuffix();
    fp=std::fopen(const_cast<char*>(this->path.c_str()),const_cast<char*>(mode.c_str()));
  }
  if(!fp){
    this->path=path;
    fp=std::fopen(const_cast<char*>(this->path.c_str()),const_cast<char*>(mode.c_str()));
  }
  if(plumed) plumed->insertFile(*this);
  plumed_massert(fp,"file " + path + "cannot be found");
  return *this;
}


bool FileBase::FileExist(const std::string& path){
  FILE *ff=NULL;
  bool do_exist=false;
  if(plumed){
    this->path=path+plumed->getSuffix();
    ff=std::fopen(const_cast<char*>(this->path.c_str()),"r");
  }
  if(!ff){
    this->path=path;
    ff=std::fopen(const_cast<char*>(this->path.c_str()),"r");
  }
  if(ff) {do_exist=true; fclose(ff);}
  if(comm) comm->Barrier();
  return do_exist; 
}

bool FileBase::isOpen(){
  bool isopen=false;
  if(fp) isopen=true;
  return isopen; 
}

void        FileBase::close(){
  plumed_assert(!cloned);
  eof=false;
  err=false;
  std::fclose(fp);
  fp=NULL;
}

FileBase::FileBase():
  fp(NULL),
  comm(NULL),
  plumed(NULL),
  action(NULL),
  cloned(false),
  eof(false),
  err(false),
  heavyFlush(false)
{
}

FileBase::~FileBase()
{
  if(plumed) plumed->eraseFile(*this);
  if(!cloned && fp) fclose(fp);
}

FileBase::operator bool()const{
  return !eof;
}

}
