/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <map>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

bool compareGrp( fecGrp* g1, fecGrp* g2 ) { return (g1->order()<g2->order()); }
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   if( _fecGrps.empty() ) {
      updateDfslist();
      fecGrp* fg=new fecGrp(findOrCreate(0),false);
      fg->addMember(findOrCreate(0),true);
      map<unsigned,CirGate *>::iterator i;
      for( i=_totalList.begin() ; i!=_totalList.end() ; ++i )
         if( i->second->getColor()==true && i->second->getType()==2 ) {
            fg->addMember(i->second,false);
            fg->addMember(i->second,true);
         }
      _fecGrps.push_back(fg);
   }
   unsigned count=0;
   unsigned fail=90;
   unsigned fecNum=_fecGrps.size();
   while(fail!=0) {
      ++count;
      for( unsigned i=0 ; i<_piList.size() ; ++i )
         _piList[i]->_sim=rnGen(4294967295);

      for( unsigned i=0 ; i<_dfsList.size() ; ++i ) {
         if( _dfsList[i]->getType()==2 ) {
               if( _dfsList[i]->getFaninId1()%2==0 ) {
                  if( _dfsList[i]->getFaninId2()%2==0 )
                     _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim)&(_dfsList[i]->getFanin1()->_sim);
                  else
                     _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim)&((_dfsList[i]->getFanin1()->_sim)^4294967295);
               }
               else {
                  if( _dfsList[i]->getFaninId2()%2==0 )
                     _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295)&(_dfsList[i]->getFanin1()->_sim);
                  else
                     _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295)
                        &((_dfsList[i]->getFanin1()->_sim)^4294967295);
               }
            }
            else if( _dfsList[i]->getType()==1 ) {
               if( _dfsList[i]->getFaninId1()%2==0 )
                  _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim);
               else
                  _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295);
            }
         }
         if( _simLog!=0 ) {
            for( int i=0 ; i<32 ; ++i ) {
               for( unsigned j=0 ; j<_piList.size() ; ++j )
                  *_simLog<<((_piList[j]->_sim>>i)&1);
               *_simLog<<" ";
               for( unsigned j=0 ; j<_poList.size() ; ++j )
                  *_simLog<<((_poList[j]->_sim>>i)&1);
               *_simLog<<endl;
            }
         }
         separateFECs();
         if( _fecGrps.size()==fecNum ) --fail;
         //else fail=18;
         fecNum=_fecGrps.size();
   }

sort(_fecGrps.begin(),_fecGrps.end(),compareGrp);
   /*for( unsigned i=0 ; i<_fecGrps.size() ; ++i ) {
      cout<<"FEC Group "<<i<<":";
      for( unsigned j=0 ; j<_fecGrps[i]->size() ; ++j ) {
         cout<<" ";
         if( _fecGrps[i]->num(j)->second ) cout<<"!";
         cout<<_fecGrps[i]->num(j)->first->getGateId();
      }
      cout<<endl;
   }*/
   cout<<"MAX_FAILS=90"<<endl;
   cout<<count*32<<" patterns simulated."<<endl;
//cout<<"There are now "<<_fecGrps.size()<<" FEC groups."<<endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   if( !patternFile.is_open() ) { cout<<"Error: cannot open file \""<<patternFile<<"\"!!"<<endl; return; }
   string input;
   unsigned count=0;
   updateDfslist();

   if( _fecGrps.empty() ) {
      fecGrp* fg=new fecGrp(findOrCreate(0),false);
      fg->addMember(findOrCreate(0),true);
      map<unsigned,CirGate *>::iterator i;
      for( i=_totalList.begin() ; i!=_totalList.end() ; ++i )
         if( i->second->getColor()==true && i->second->getType()==2 ) {
            fg->addMember(i->second,false);
            fg->addMember(i->second,true);
         }
      /*for( unsigned i=0 ; i<_dfsList.size() ; ++i ) {
         if( _dfsList[i]->getType()==2 ) {
            fg->addMember(_dfsList[i],false);
            fg->addMember(_dfsList[i],true);
         }
      }*/
      _fecGrps.push_back(fg);
   }

   while( patternFile>>input ) {
      if( input.size()!=_piList.size() ) {
         cout<<"Error: Pattern("<<input<<") length("<<input.size()<<" does not match the number of inputs("
             <<_piList.size()<<") in a circuit!!";
         count=0;
         break;
      }
      char* bit=new char[input.size()+1];
      strcpy(bit,input.c_str());
      if( count%32==0 )
         for( unsigned i=0 ; i<_piList.size() ; ++i )
            _piList[i]->_sim=0;
      ++count;

      for( unsigned i=0 ; i<input.size() ; ++i ) {
         if( bit[i]=='1' ) {
            _piList[i]->_sim=((_piList[i]->_sim)|(1<<((count-1)%32)));
//cout<<(_piList[i]->_sim)<<" "<<(1<<((count-1)%32))<<endl;
         }
         else if( bit[i]!='0' ) {
            cout<<"Error: Pattern("<<input<<") contains a non-0/1 character('"<<bit[i]<<"')."<<endl;
            count=0; break;
         }
      }
      if( count==0 ) break;
      if( count%32==0 ) {
         for( unsigned i=0 ; i<_dfsList.size() ; ++i ) {
            if( _dfsList[i]->getType()==2 ) {
               if( _dfsList[i]->getFaninId1()%2==0 ) {
                  if( _dfsList[i]->getFaninId2()%2==0 )
                     _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim)&(_dfsList[i]->getFanin1()->_sim);
                  else
                     _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim)&((_dfsList[i]->getFanin1()->_sim)^4294967295);
               }
               else {
                  if( _dfsList[i]->getFaninId2()%2==0 )
                     _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295)&(_dfsList[i]->getFanin1()->_sim);
                  else
                     _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295)
                        &((_dfsList[i]->getFanin1()->_sim)^4294967295);
               }
            }
            else if( _dfsList[i]->getType()==1 ) {
               if( _dfsList[i]->getFaninId1()%2==0 )
                  _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim);
               else
                  _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295);
            }
//cout<<"Gate "<<_dfsList[i]->getGateId()<<": "<<_dfsList[i]->_sim<<endl;
         }
         if( _simLog!=0 ) {
            for( int i=0 ; i<32 ; ++i ) {
               for( unsigned j=0 ; j<_piList.size() ; ++j )
                  *_simLog<<((_piList[j]->_sim>>i)&1);
               *_simLog<<" ";
               for( unsigned j=0 ; j<_poList.size() ; ++j )
                  *_simLog<<((_poList[j]->_sim>>i)&1);
               *_simLog<<endl;
            }
         }
         separateFECs();
         //for( unsigned i=0 ; i<_piList.size() ; ++i )
         //    _piList[i]->_sim=0;
      }
   }
   if( count%32!=0 ) {
      for( unsigned i=0 ; i<_dfsList.size() ; ++i ) {
         if( _dfsList[i]->getType()==2 ) {
            if( _dfsList[i]->getFaninId1()%2==0 ) {
               if( _dfsList[i]->getFaninId2()%2==0 )
                  _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim)&(_dfsList[i]->getFanin1()->_sim);
               else
                  _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim)&((_dfsList[i]->getFanin1()->_sim)^4294967295);
            }
            else {
               if( _dfsList[i]->getFaninId2()%2==0 )
                  _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295)&(_dfsList[i]->getFanin1()->_sim);
               else
                  _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295)
                     &((_dfsList[i]->getFanin1()->_sim)^4294967295);
            }
         }
         else if( _dfsList[i]->getType()==1 ) {
            if( _dfsList[i]->getFaninId1()%2==0 )
               _dfsList[i]->_sim=(_dfsList[i]->getFanin0()->_sim);
            else
               _dfsList[i]->_sim=((_dfsList[i]->getFanin0()->_sim)^4294967295);
         }
//cout<<"Gate "<<_dfsList[i]->getGateId()<<": "<<_dfsList[i]->_sim<<endl;
      }
      if( _simLog!=0 ) {
         for( int i=0 ; i<32 ; ++i ) {
            for( unsigned j=0 ; j<_piList.size() ; ++j )
               *_simLog<<((_piList[j]->_sim>>i)&1);
            *_simLog<<" ";
            for( unsigned j=0 ; j<_poList.size() ; ++j )
               *_simLog<<((_poList[j]->_sim>>i)&1);
            *_simLog<<endl;
         }
      }
      separateFECs();
   }
   sort(_fecGrps.begin(),_fecGrps.end(),compareGrp);
   /*for( unsigned i=0 ; i<_fecGrps.size() ; ++i ) {
      cout<<"FEC Group "<<i<<":";
      for( unsigned j=0 ; j<_fecGrps[i]->size() ; ++j ) {
         cout<<" ";
         if( _fecGrps[i]->num(j)->second ) cout<<"!";
         cout<<_fecGrps[i]->num(j)->first->getGateId();
      }
      cout<<endl;
   }*/
   cout<<count<<" patterns simulated."<<endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::separateFECs()
{
   vector<fecGrp*> newFecGrps;                                       //a new set of FEC groups to swap

   for( unsigned i=0 ; i<_fecGrps.size() ; ++i ) {                   //for the same FEC group
      HashMap<simValue,fecGrp*> fecMap(getHashSize(_fecGrps[i]->size()));
      for( unsigned j=0 ; j<_fecGrps[i]->size() ; ++j ) {
         fecGrp* fg=NULL;                                            //insert a candidate
         if(_fecGrps[i]->num(j)->second) {
            fecGrp *g=new fecGrp(_fecGrps[i]->num(j)->first,_fecGrps[i]->num(j)->second);
//cout<<"!"<<_fecGrps[i]->num(j)->first->getGateId()<<" "<<((_fecGrps[i]->num(j)->first->_sim)^4294967295);
            fg=fecMap.checkAndInsert(simValue((_fecGrps[i]->num(j)->first->_sim)^4294967295),g);
         }
         else {
            fecGrp *g=new fecGrp(_fecGrps[i]->num(j)->first,_fecGrps[i]->num(j)->second);
//cout<<_fecGrps[i]->num(j)->first->getGateId()<<" "<<_fecGrps[i]->num(j)->first->_sim;
            fg=fecMap.checkAndInsert(simValue(_fecGrps[i]->num(j)->first->_sim),g);
         }

         if(fg!=NULL)
            fg->addMember(_fecGrps[i]->num(j)->first,_fecGrps[i]->num(j)->second);
      }
      for( unsigned j=0 ; j<fecMap.numBuckets() ; ++j ) {
         for( unsigned k=0 ; k<fecMap[j].size() ; ++k ) {
//cout<<j<<" "<<k<<" "<<fecMap[j][k].second->size()<<endl;
            if( fecMap[j][k].second->size()>1 ) {
//cout<<"add "<<fecMap[j][k].second->num(0)->first->getGateId();
               bool flag=false;
               for( unsigned l=0 ; l<fecMap[j][k].second->size() ; ++l )
                  if( fecMap[j][k].second->num(l)->second==false ) {
                     flag=true; break;
                  }
               if(flag)
                  newFecGrps.push_back(fecMap[j][k].second);
            }
            else
               delete fecMap[j][k].second;
         }
      }
      fecMap.clear();
   }
   for( unsigned i=0 ; i<_fecGrps.size() ; ++i )
      delete _fecGrps[i];
   _fecGrps.swap(newFecGrps);
}
