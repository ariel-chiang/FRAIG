/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   /*if( _isSweep ) return;
   else _isSweep=true;

   if( _poList.empty() ) {
      vector<unsigned> _eraseId;
      map<unsigned,CirGate*>::iterator i=_totalList.begin();
      for( i=_totalList.begin() ; i!=_totalList.end() ; ++i )
         if( i->second->getType()!=0 ) {
            cout<<"Sweeping: "<<i->second->getTypeStr()<<"("<<i->first<<") removed..."<<endl;
            delete i->second;
            _eraseId.push_back(i->first);
         }
      for( unsigned j=0 ; j<_eraseId.size() ; ++j )
            _totalList.erase(_eraseId[j]);
      return;
   }*/

   /*if( !_poList[0]->getColor() ) {
      ofstream o;
      outputNetlist(o,false);
   }*/
   /*if( !_dfsColor ) {
      ofstream o;
      outputNetlist(o,false);
   }*/

   updateDfslist();
   vector<unsigned> _eraseId;
   map<unsigned,CirGate*>::iterator i;
   for( i=_totalList.begin() ; i!=_totalList.end() ; ++i ) {
      if( i->second->getType()!=0 && i->second->getColor()==false ) {
         if( i->second->getType()==3 ) continue;
         cout<<"Sweeping: "<<i->second->getTypeStr()<<"("<<i->first<<") removed..."<<endl;
         if( i->second->getType()==2 ) {
            --A;
            i->second->_faninList[0]->deleteFanout(i->second);
            i->second->_faninList[1]->deleteFanout(i->second);
         }
         delete i->second;
         _eraseId.push_back(i->first);
      }
   }
   for( unsigned j=0 ; j<_eraseId.size() ; ++j )
         _totalList.erase(_eraseId[j]);
   /*for( i=_totalList.begin() ; i!=_totalList.end() ; ++i )
      i->second->setColor(false);*/
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   /*map<unsigned,CirGate*>::iterator i;
   for( i=_totalList.begin() ; i!=_totalList.end() ; ++i ) {
      cout<<i->first<<"-"<<i->second->getColor()<<" ";
      cout<<i->second->getFaninId1()<<" ";
      cout<<i->second->getFanoutId()<<endl;
   }*/
   bool flag;
   if( !_poList.empty() ) flag=_poList[0]->getColor();
   vector<unsigned>* eraseId=new vector<unsigned>;
   CirGate* const0=findOrCreate(0);
   for( unsigned i=0 ; i<_poList.size() ; ++i )
      _poList[i]->trivialOpt(flag,eraseId,const0);
   for( unsigned i=0 ; i<eraseId->size() ; ++i ) {
      map<unsigned,CirGate*>::iterator j=_totalList.find(eraseId->operator[](i));
      delete j->second;
      _totalList.erase(j);
      --A;
      //cout<<eraseId->operator[](i)<<" ";
   }
   delete eraseId;
   //_isSweep=false;
   resetColor();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
