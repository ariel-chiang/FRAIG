/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/

/*CIRGate <(int gateId)> : print gateType, gateId, gateName, and line*/
void
CirGate::reportGate() const
{
   cout<<"=================================================="<<endl<<"="<<" ";
   int l=37;
   switch(_type) {
   case 0: cout<<"PI"; l-=2; break;
   case 1: cout<<"PO"; l-=2; break;
   case 2: cout<<"AIG"; l-=3; break;
   case 3: cout<<"CONST"; l-=5; break;
   case 4: cout<<"UNDEF"; l-=5; break;
   }
   cout<<"("<<_gateId<<")";
   unsigned i=_gateId;
   while( i/10!=0 ) { i=i/10; --l; }
   i=_line;
   while( i/10!=0 ) { i=i/10; --l; }
   if(!_name.empty()) { cout<<"\""<<_name<<"\""; l=l-_name.size()-2; }
   cout<<", line "<<_line<<setw(l)<<right<<"="<<endl;
   cout<<"= FECs:";
   bool flag=false;
   for( unsigned i=0 ; i<cirMgr->_fecGrps.size() ; ++i ) {
//cout<<"try "<<cirMgr->_fecGrps[i]->num(0)->first->getGateId()<<endl;
      if( _gateId<cirMgr->_fecGrps[i]->num(0)->first->getGateId() ) break;
      else if( _gateId==cirMgr->_fecGrps[i]->num(0)->first->getGateId() ) {
         if( cirMgr->_fecGrps[i]->num(0)->second ) {
            for( unsigned k=1 ; k<cirMgr->_fecGrps[i]->size() ; ++k ) {
               cout<<" "; if( !cirMgr->_fecGrps[i]->num(k)->second ) cout<<"!";
               cout<<cirMgr->_fecGrps[i]->num(k)->first->getGateId();
            }
         }
         else {
            for( unsigned k=1 ; k<cirMgr->_fecGrps[i]->size() ; ++k ) {
               cout<<" "; if( cirMgr->_fecGrps[i]->num(k)->second ) cout<<"!";
               cout<<cirMgr->_fecGrps[i]->num(k)->first->getGateId();
            }
         }
         break;
      }
      for( unsigned j=1 ; j<cirMgr->_fecGrps[i]->size() ; ++j ) {
//cout<<"try "<<cirMgr->_fecGrps[i]->num(1)->first->getGateId()<<endl;
         if( _gateId<cirMgr->_fecGrps[i]->num(j)->first->getGateId() ) break;
         else if( _gateId==cirMgr->_fecGrps[i]->num(j)->first->getGateId() ) {
            if( cirMgr->_fecGrps[i]->num(j)->second ) {
               for( unsigned k=0 ; k<cirMgr->_fecGrps[i]->size() ; ++k ) {
                  if( k==j ) continue;
                  cout<<" "; if( !cirMgr->_fecGrps[i]->num(k)->second ) cout<<"!";
                  cout<<cirMgr->_fecGrps[i]->num(k)->first->getGateId();
               }
            }
            else {
               for( unsigned k=0 ; k<cirMgr->_fecGrps[i]->size() ; ++k ) {
                  if( k==j ) continue;
                  cout<<" "; if( cirMgr->_fecGrps[i]->num(k)->second ) cout<<"!";
                  cout<<cirMgr->_fecGrps[i]->num(k)->first->getGateId();
               }
            }
            flag=true; break;
         }
      }
      if( flag ) break;
   }
   cout<<" ="<<endl<<"= Value: ";
   for( int i=31 ; i>=0 ; i-- ) { cout<<((_sim>>i)&1); if( i%4==0 && i!=0 ) cout<<"_"; }
   cout<<" ="<<endl<<"=================================================="<<endl;
}

/*CIRGate <<(int gateId)> [<-FANIn> <(int level)>]> : print fanins in DFS to a certain level*/
void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   trackFanin(level,0,_color2,this,false);
   trackFanin(level,0,_color2,this,false);
}

/*CIRGate <<(int gateId)> [<-FANOut> <(int level)>]> : print fanouts in DFS to a certain level*/
void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   trackFanout(level,0,_color2,this,false);
   trackFanout(level,0,_color2,this,false);
}

/*level: how many levels to print, depth: recent level, flag: print or not, g: recent gate, isInvert: if g is inverted*/
void
CirGate::trackFanin(int& level, int depth, bool& flag, CirGate *g, bool isInvert) {
   if(level<depth) return;
   
   if(flag) {
      for( int i=0 ; i<depth ; ++i ) cout<<"  ";
      if( isInvert==true ) cout<<"!";
      switch(g->getType()) {
         case 0: cout<<"PI "<<g->getGateId(); break;
         case 1: cout<<"PO "<<g->getGateId(); break;
         case 2: cout<<"AIG "<<g->getGateId(); break;
         case 3: cout<<"CONST "<<g->getGateId(); break;
         case 4: cout<<"UNDEF "<<g->getGateId(); break;
      }
   }
   if(g->_color2!=flag) { if(flag) cout<<" (*)"<<endl; return; }
   else if(flag) cout<<endl;

   for( unsigned i=0 ; i<g->_faninList.size() ; ++i )
      /*if( g->_faninList[i]->getColor()==flag )*/
         trackFanin(level,depth+1,flag,g->_faninList[i],g->_isInvert[i]);
   if( /*!g->_faninList.empty() &&*/ level!=depth ) g->_color2=!flag;
}

/*level: how many levels to print, depth: recent level, flag: print or not, g: recent gate, isInvert: if g is inverted*/
void
CirGate::trackFanout(int& level, int depth, bool& flag, CirGate *g, bool isInvert) {
   if(level<depth) return;

   if(flag) {
      for( int i=0 ; i<depth ; ++i ) cout<<"  ";
      if( isInvert==true ) cout<<"!";
      switch(g->getType()) {
         case 0: cout<<"PI "<<g->getGateId(); break;
         case 1: cout<<"PO "<<g->getGateId(); break;
         case 2: cout<<"AIG "<<g->getGateId(); break;
         case 3: cout<<"CONST "<<g->getGateId(); break;
         case 4: cout<<"UNDEF "<<g->getGateId(); break;
      }
   }
   if(g->_color2!=flag) { if(flag) cout<<" (*)"<<endl; return; }
   else if(flag) cout<<endl;

   for( unsigned i=0 ; i<g->_fanoutList.size() ; ++i ) {
      for( unsigned j=0 ; j<g->_fanoutList[i]->_faninList.size() ; ++j )
         if( g->_fanoutList[i]->_faninList[j]->getGateId() == g->getGateId() ) {
            isInvert=g->_fanoutList[i]->_isInvert[j]; break;
         }
      trackFanout(level,depth+1,flag,g->_fanoutList[i],isInvert);
   }
   if( /*!g->_fanoutList.empty() &&*/ level!=depth ) g->_color2=!flag;
}

/*flag: original color of all gates, count: record how many gates have been printed*/
bool
CirGate::printFanin(bool& flag, unsigned& count)
{
   if( _color==flag ) {
      _color=!flag;
      for( unsigned i=0 ; i<_faninList.size() ; ++i )
      if( _faninList[i]->printFanin(flag,count) )
         ++count;
      switch(_type) {
         case 0: cout<<"["<<count<<"] "<<"PI  "<<_gateId;
                 if( !_name.empty() ) cout<<" ("<<_name<<")";
                 cout<<endl; break;
         case 1: cout<<"["<<count<<"] "<<"PO  "<<_gateId<<" ";
                 if( _faninList[0]->getType() == Undef ) cout<<"*";
                 if( _isInvert[0]==true ) cout<<"!";
                 cout<<_faninList[0]->getGateId();
                 if( !_name.empty() ) cout<<" ("<<_name<<")";
                 cout<<endl; ++count; break;
         case 2: cout<<"["<<count<<"] "<<"AIG "<<_gateId<<" ";
                 if( _faninList[0]->getType() == Undef ) cout<<"*";
                 if( _isInvert[0]==true ) cout<<"!";
                 cout<<_faninList[0]->getGateId()<<" ";
                 if( _faninList[1]->getType() == Undef ) cout<<"*";
                 if( _isInvert[1]==true ) cout<<"!";
                 cout<<_faninList[1]->getGateId()<<endl; break;
         case 3: cout<<"["<<count<<"] "<<"CONST"<<_gateId<<endl; break;
         case 4: break;
      }
      return true;
   }
   else
      return false;
}

/*flag: original color, outfile: output file, count: count how many aigs there are, toPrint: to print or not*/
void
CirGate::outputFanin(bool& flag, ostream& outfile, unsigned& count, bool& toPrint)
{
   if( _color==flag ) {
      _color=!flag;
      for( unsigned i=0 ; i<_faninList.size() ; ++i )
         _faninList[i]->outputFanin(flag,outfile,count,toPrint);
      switch(_type) {
         case 2: if(toPrint) {
                    outfile<<_gateId*2<<" ";
                    if( _isInvert[0]==true )
                       outfile<<_faninList[0]->getGateId()*2+1<<" ";
                    else
                       outfile<<_faninList[0]->getGateId()*2<<" ";
                    if( _isInvert[1]==true )
                       outfile<<_faninList[1]->getGateId()*2+1<<endl;
                    else
                       outfile<<_faninList[1]->getGateId()*2<<endl;
                 }
                 ++count;
         default: break;
      }
   }
}

void
CirGate::trivialOpt(bool& flag, vector<unsigned>* eraseId, CirGate* const0)
{
   if( _color==flag ) {
      _color=!flag;
      for( unsigned i=0 ; i<_faninList.size() ; ++i )
         _faninList[i]->trivialOpt(flag,eraseId,const0);
      switch(_type) {
         case 2: if( _faninList[0]->getGateId()==0 ) {
                    eraseId->push_back(_gateId);
                    const0->deleteFanout(this);
                    _faninList[1]->deleteFanout(this);
                    if( _isInvert[0]==true ) {
                       string str=(_isInvert[1]==true)?"!":"";
                       cout<<"Simplifying: "<<_faninList[1]->getGateId()<<" merging "<<str<<_gateId<<"..."<<endl;
                       //cout<<"delete fo "<<_gateId<<" for "<<_faninList[1]->getGateId()<<endl;
                       for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
                          _faninList[1]->addFanout(_fanoutList[i]);
                          //cout<<"add fo "<<_fanoutList[i]->getGateId()<<" for "<<_faninList[1]->getGateId()<<endl;
                          if( _fanoutList[i]->getFaninId1()/2 == _gateId )
                             _fanoutList[i]->setFaninId1(_faninList[1],_isInvert[1]);
                          else
                             _fanoutList[i]->setFaninId2(_faninList[1],_isInvert[1]);
                          //cout<<"set fi "<<_faninList[1]->getGateId()<<" for "<<_fanoutList[i]->getGateId()<<endl;
                       }
                    }
                    else {
                       cout<<"Simplifying: 0 merging "<<_gateId<<"..."<<endl;
                       for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
                          const0->addFanout(_fanoutList[i]);
                          if( _fanoutList[i]->getFaninId1()/2 == _gateId )
                             _fanoutList[i]->setFaninId1(const0,false);
                          else
                             _fanoutList[i]->setFaninId2(const0,false);
                       }
                    }
                 }
                 else if( _faninList[1]->getGateId()==0 ) {
                    eraseId->push_back(_gateId);
                    const0->deleteFanout(this);
                    _faninList[0]->deleteFanout(this);
                    if( _isInvert[1]==true ) {
                       string str=(_isInvert[0]==true)?"!":"";
                       cout<<"Simplifying: "<<_faninList[0]->getGateId()<<" merging "<<str<<_gateId<<"..."<<endl;
                       for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
                          _faninList[0]->addFanout(_fanoutList[i]);
                          if( _fanoutList[i]->getFaninId1()/2 == _gateId )
                             _fanoutList[i]->setFaninId1(_faninList[0],_isInvert[0]);
                          else
                             _fanoutList[i]->setFaninId2(_faninList[0],_isInvert[0]);
                       }
                    }
                    else {
                       cout<<"Simplifying: 0 merging "<<_gateId<<"..."<<endl;
                       for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
                          const0->addFanout(_fanoutList[i]);
                          if( _fanoutList[i]->getFaninId1()/2 == _gateId )
                             _fanoutList[i]->setFaninId1(const0,false);
                          else
                             _fanoutList[i]->setFaninId2(const0,false);
                       }
                    }
                 }
                 else if ( _faninList[0]->getGateId()==_faninList[1]->getGateId() ) {
                    eraseId->push_back(_gateId);
                    _faninList[0]->deleteFanout(this);
                    _faninList[1]->deleteFanout(this);
                    if( _isInvert[0]!=_isInvert[1] ) {
                       cout<<"Simplifying: 0 merging "<<_gateId<<"..."<<endl;
                       for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
                          const0->addFanout(_fanoutList[i]);
                          if( _fanoutList[i]->getFaninId1()/2 == _gateId )
                             _fanoutList[i]->setFaninId1(const0,false);
                          else
                             _fanoutList[i]->setFaninId2(const0,false);
                       }
                    }
                    else {
                       string str=(_isInvert[0]==true)?"!":"";
                       cout<<"Simplifying: "<<_faninList[0]->getGateId()<<" merging "<<str<<_gateId<<"..."<<endl;
                       for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
                          _faninList[0]->addFanout(_fanoutList[i]);
                          if( _fanoutList[i]->getFaninId1()/2 == _gateId )
                             _fanoutList[i]->setFaninId1(_faninList[0],_isInvert[0]);
                          else
                             _fanoutList[i]->setFaninId2(_faninList[0],_isInvert[0]);
                       }
                    }
                 }
                 break;
         default: break;
      }
   }
}

unsigned
CirGate::merge(CirGate *g2)
{
   for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
      g2->addFanout(_fanoutList[i]);
      if( _fanoutList[i]->getFaninId1()/2==_gateId )
         _fanoutList[i]->setFaninId1(g2,false);
      else
         _fanoutList[i]->setFaninId2(g2,false);
   }
   _faninList[0]->deleteFanout(this);
   _faninList[1]->deleteFanout(this);
   return _gateId;
}

unsigned
CirGate::mergeInv(CirGate *g2)
{
   for( unsigned i=0 ; i<_fanoutList.size() ; ++i ) {
      g2->addFanout(_fanoutList[i]);
      if( _fanoutList[i]->getFaninId1()/2==_gateId )
         _fanoutList[i]->setFaninId1(g2,true);
      else
         _fanoutList[i]->setFaninId2(g2,true);
   }
   _faninList[0]->deleteFanout(this);
   _faninList[1]->deleteFanout(this);
   return _gateId;
}
