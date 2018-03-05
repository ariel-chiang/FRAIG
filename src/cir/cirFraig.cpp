/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   updateDfslist();
   /*for( unsigned i=0 ; i<_dfsList.size() ; ++i )
      cout<<_dfsList[i]->getGateId()<<" ";
   cout<<endl;*/
   
   HashMap<HashKey,CirGate*> hashMap(getHashSize(A));
   for( unsigned i=0 ; i<_dfsList.size() ; ++i ) {
      if( _dfsList[i]->getType()!=2 ) continue;
      HashKey key(_dfsList[i]->getFaninId1(), _dfsList[i]->getFaninId2() );
      CirGate* g=hashMap.checkAndInsert(key,_dfsList[i]);
//cout<<"gate "<<_dfsList[i]->getGateId()<<" "<<_dfsList[i]->getFaninId1()<<" "<<_dfsList[i]->getFaninId2()<<" "<<g<<endl;
      if( g!=NULL ) {
         cout<<"Strashing: ";
         replace(_dfsList[i],g);
      }
   }

   updateDfslist();
   /*for( unsigned i=0 ; i<_dfsList.size() ; ++i )
      cout<<_dfsList[i]->getGateId()<<" ";
   cout<<endl;*/
}

void
CirMgr::fraig()
{
   Var var0;
   if( _solver==0 ) {
      _solver=new SatSolver();
      _solver->initialize();
      for( unsigned i=0 ; i<_dfsList.size() ; ++i )
         if( _dfsList[i]->_type != 1 )
            _dfsList[i]->_var=_solver->newVar();
      CirGate* g=findOrCreate(0);
      var0=g->_var=_solver->newVar();
   }

   for( unsigned i=0 ; i<_dfsList.size() ; ++i )
      if( _dfsList[i]->_type == 2 )
         _solver->addAigCNF(_dfsList[i]->_var,_dfsList[i]->_faninList[0]->_var,_dfsList[i]->_isInvert[0],
                            _dfsList[i]->_faninList[1]->_var,_dfsList[i]->_isInvert[1]);

   for( unsigned i=0 ; i<_fecGrps.size() ; ++i ) {
      if( _fecGrps[i]->_member[0]->second ) continue;
      for( unsigned j=1 ; j<_fecGrps[i]->_member.size() ; ++j ) {
         Var newV=_solver->newVar();
         _solver->addXorCNF(newV,_fecGrps[i]->_member[0]->first->_var,_fecGrps[i]->_member[0]->second,
                           _fecGrps[i]->_member[j]->first->_var,_fecGrps[i]->_member[j]->second);
         _solver->assumeRelease();
         _solver->assumeProperty(newV,true);
         _solver->assumeProperty(var0,false);
         bool result=_solver->assumpSolve();
         if(!result) {
            /*cout<<_fecGrps[i]->_member[0]->first->_gateId<<" "<<_fecGrps[i]->_member[j]->first->_gateId<<" are the same"<<endl;*/
            cout<<"Fraig: ";
            if( _fecGrps[i]->_member[j]->second )
               replaceInv(_fecGrps[i]->_member[j]->first,_fecGrps[i]->_member[0]->first);
            else
               replace(_fecGrps[i]->_member[j]->first,_fecGrps[i]->_member[0]->first);
         }
         //else {
            /*cout<<_fecGrps[i]->_member[0]->first->_gateId<<" "<<_fecGrps[i]->_member[j]->first->_gateId<<" are different ";
            for( unsigned k=0 ; k<_piList.size() ; ++k )
               cout<<_solver->getValue(_fecGrps[k]->_member[0]->first->_var);cout<<endl;*/
         //}
      }
   }

   for( unsigned i=0 ; i<_fecGrps.size() ; ++i )
      delete _fecGrps[i];
   
   _fecGrps.clear();
   delete _solver;
   _solver=0;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::replace(CirGate* g1, CirGate* g2) {
   unsigned id=g1->merge(g2);
   cout<<g2->getGateId()<<" merging "<<g1->getGateId()<<"..."<<endl;
   map<unsigned,CirGate*>::iterator j=_totalList.find(id);
   delete j->second;
   _totalList.erase(j);
   --A;
}
void
CirMgr::replaceInv(CirGate* g1, CirGate* g2) {
   unsigned id=g1->mergeInv(g2);
   cout<<g2->getGateId()<<" merging !"<<g1->getGateId()<<"..."<<endl;
   map<unsigned,CirGate*>::iterator j=_totalList.find(id);
   delete j->second;
   _totalList.erase(j);
   --A;
}
