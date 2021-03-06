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
#include "VesselRegister.h"
#include "StoreValueVessel.h"
#include "ActionWithVessel.h"

namespace PLMD {
namespace vesselbase{

// This is the most efficient implementation 
// The calculation of all the colvars is parallelized 
// but the loops for calculating moments are not
// Feel free to reimplement this if you know how
class Moments : public StoreValueVessel {
private:
  std::vector<unsigned> powers;
  std::vector<Value*> value_out;
public:
  static void registerKeywords( Keywords& keys );
  static void reserveKeyword( Keywords& keys );
  Moments( const vesselbase::VesselOptions& da );
  std::string description();
  void resize();
  void finish();
  bool applyForce( std::vector<double>& forces );
};

PLUMED_REGISTER_VESSEL(Moments,"MOMENTS")

void Moments::registerKeywords( Keywords& keys ){
  StoreValueVessel::registerKeywords( keys );
}

void Moments::reserveKeyword( Keywords& keys ){
  keys.reserve("optional","MOMENTS","calculate the moments of the distribution of collective variables. " 
  "The \\f$m\\f$th moment of a distribution is calculated using \\f$\\frac{1}{N} \\sum_{i=1}^N ( s_i - \\overline{s} )^m \\f$, where \\f$\\overline{s}\\f$ is "
  "the average for the distribution. The moments keyword takes a lists of integers as input or a range. Each integer is a value of \\f$m\\f$. The final "
  "calculated values can be referenced using moment-\\f$m\\f$.");  
}

Moments::Moments( const vesselbase::VesselOptions& da) :
StoreValueVessel(da)
{
   ActionWithValue* a=dynamic_cast<ActionWithValue*>( getAction() );
   plumed_massert(a,"cannot create passable values as base action does not inherit from ActionWithValue");

   std::vector<std::string> moments=Tools::getWords(getAllInput(),"\t\n ,"); 
   Tools::interpretRanges(moments); unsigned nn;
   for(unsigned i=0;i<moments.size();++i){
       a->addComponentWithDerivatives( "moment-" + moments[i] );  
       a->componentIsNotPeriodic( "moment-" + moments[i] );
       value_out.push_back( a->copyOutput( a->getNumberOfComponents()-1 ) );
       Tools::convert( moments[i], nn );
       if( nn<2 ) error("moments are only possible for m>=2" );
       powers.push_back( nn ); std::string num; Tools::convert(powers[i],num);
   }
}

void Moments::resize(){
   StoreDataVessel::resize();
   unsigned nder=getAction()->getNumberOfDerivatives();
   for(unsigned i=0;i<value_out.size();++i) value_out[i]->resizeDerivatives( nder );
}

std::string Moments::description(){
   std::string descri, num;
   Tools::convert(powers[0],num);
   descri = "value " + getAction()->getLabel() + "." + "moment-" + num + " contains the " + num + "th moment of the distribution";
   for(unsigned i=1;i<powers.size();++i){
       Tools::convert(powers[i],num);
       descri = descri + "\n  value " + getAction()->getLabel() + "." + "moment-" + num + " contains the " + num + "th moment of the distribution";
   }
   return descri;
}

void Moments::finish(){
  StoreDataVessel::finish();

  const double pi=3.141592653589793238462643383279502884197169399375105820974944592307;
  unsigned nvals=getAction()->getFullNumberOfTasks(); 

  double mean=0; Value myvalue;
  if( getAction()->isPeriodic() ){
     std::string str_min, str_max; getAction()->retrieveDomain( str_min, str_max );
     double pfactor, min, max; Tools::convert(str_min,min); Tools::convert(str_max,max);
     pfactor = 2*pi / ( max-min ); myvalue.setDomain( str_min, str_max );
     double sinsum=0, cossum=0, val;
     for(unsigned i=0;i<nvals;++i){ val=pfactor*( getValue(i) - min ); sinsum+=sin(val); cossum+=cos(val); }
     mean = 0.5 + atan2( sinsum / static_cast<double>( nvals ) , cossum / static_cast<double>( nvals ) ) / (2*pi);
     mean = min + (max-min)*mean;
  } else {
     for(unsigned i=0;i<nvals;++i) mean+=getValue(i);    
     mean/=static_cast<double>( nvals ); myvalue.setNotPeriodic();
  }

  for(unsigned npow=0;npow<powers.size();++npow){
     double dev1=0; 
     for(unsigned i=0;i<nvals;++i) dev1+=pow( myvalue.difference( mean, getValue(i) ), powers[npow] - 1 ); 
     dev1/=static_cast<double>( nvals );

     std::vector<double> pref(1); double tmp, moment=0; 
     for(unsigned i=0;i<nvals;++i){
         tmp=myvalue.difference( mean, getValue(i) );
         pref[0]=pow( tmp, powers[npow] - 1 ) - dev1;
         moment+=pow( tmp, powers[npow] );
         if( usingLowMem() ){
            recompute( i, 0 ); // Not very efficient 
            chainRule( 0, pref, value_out[npow] );
         } else {
            chainRule( i, pref, value_out[npow] );
         }
     }
     value_out[npow]->chainRule( powers[npow] / static_cast<double>( nvals ) );
     value_out[npow]->set( moment / static_cast<double>( nvals ) ); 
  }
}

bool Moments::applyForce( std::vector<double>& forces ){
  std::vector<double> tmpforce( forces.size() );
  forces.assign(forces.size(),0.0); bool wasforced=false;
  for(unsigned i=0;i<value_out.size();++i){
     if( value_out[i]->applyForce( tmpforce ) ){
         wasforced=true;
         for(unsigned j=0;j<forces.size();++j) forces[j]+=tmpforce[j];
     }
  }
  return wasforced;
}

}
}
