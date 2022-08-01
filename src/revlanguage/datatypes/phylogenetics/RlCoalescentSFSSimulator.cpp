#include "CoalescentSFSSimulator.h"

#include <ostream>
#include <string>
#include <vector>

#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "Delimiter.h"
#include "RevObject.h"
#include "Real.h"
#include "RlCoalescentSFSSimulator.h"
#include "ModelVector.h"
#include "Natural.h"
#include "RlBoolean.h"
#include "RealPos.h"
#include "RlString.h"
#include "RlMatrixRealPos.h"
#include "RlTimeTree.h"
#include "TypeSpec.h"
#include "MemberProcedure.h"
#include "MethodTable.h"
#include "RevPtr.h"
#include "RevVariable.h"
#include "WorkspaceToCoreWrapperObject.h"

namespace RevLanguage { class Argument; }


using namespace RevLanguage;

CoalescentSFSSimulator::CoalescentSFSSimulator() : WorkspaceToCoreWrapperObject<RevBayesCore::CoalescentSFSSimulator>()
{
    
    // simulating an SFS
    
    ArgumentRules* tpv_arg_rules = new ArgumentRules();
    
    tpv_arg_rules->push_back( new ArgumentRule( "sampleSize"    , Natural::getClassTypeSpec(), "The sample size, i.e., number of individuals at present.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
    tpv_arg_rules->push_back( new ArgumentRule( "reps"          , Natural::getClassTypeSpec(), "The number of replicate to simulate the frequencies.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );

    methods.addFunction(new MemberProcedure( "simulateSFS", ModelVector< Natural >::getClassTypeSpec(), tpv_arg_rules) );

    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
CoalescentSFSSimulator* CoalescentSFSSimulator::clone(void) const
{
    
    return new CoalescentSFSSimulator(*this);
}


void CoalescentSFSSimulator::constructInternalObject( void )
{
    // we free the memory first
    delete value;
    
    // get the parameter values
    std::vector<double> ps  = static_cast<const ModelVector<RealPos> &>( population_sizes->getRevObject() ).getValue();
    std::vector<double> cp  = static_cast<const ModelVector<RealPos> &>( change_points->getRevObject() ).getValue();
    double gt               = static_cast<const RealPos &>( generation_time->getRevObject() ).getValue();
    double mr               = static_cast<const RealPos &>( mutation_rate->getRevObject() ).getValue();
    
    value = new RevBayesCore::CoalescentSFSSimulator(ps, cp, gt, mr);

    
}


/* Map calls to member methods */
RevPtr<RevVariable> CoalescentSFSSimulator::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{
    
    if (name == "simulateSFS")
    {
        found = true;
            
        long sample_size    = static_cast<const Natural &>( args[0].getVariable()->getRevObject() ).getValue();
        long reps           = static_cast<const Natural &>( args[1].getVariable()->getRevObject() ).getValue();

        RevBayesCore::RbVector<long>* m = value->simulateSFS(sample_size, reps);
        
        return new RevVariable( new ModelVector<Natural>( *m ) );
    }
    
    return WorkspaceToCoreWrapperObject<RevBayesCore::CoalescentSFSSimulator>::executeMethod( name, args, found );
}


/** Get Rev type of object */
const std::string& CoalescentSFSSimulator::getClassType(void)
{
    
    static std::string rev_type = "CoalescentSFSSimulator";
    
    return rev_type;
}

/** Get class type spec describing type of object */
const TypeSpec& CoalescentSFSSimulator::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( WorkspaceToCoreWrapperObject<RevBayesCore::CoalescentSFSSimulator>::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the Rev name for the constructor function.
 *
 * \return Rev name of constructor function.
 */
std::string CoalescentSFSSimulator::getConstructorFunctionName( void ) const
{
    // create a constructor function name variable that is the same for all instance of this class
    std::string c_name = "CoalescentSFSSimulator";
    
    return c_name;
}


/** Return member rules (no members) */
const MemberRules& CoalescentSFSSimulator::getParameterRules(void) const
{
    
    static MemberRules argument_rules;
    static bool rules_set = false;
    
    if ( rules_set == false )
    {
        
        argument_rules.push_back( new ArgumentRule( "populationSizes"   , ModelVector<RealPos>::getClassTypeSpec(), "The (constant) population sizes per interval.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        argument_rules.push_back( new ArgumentRule( "changePoints"      , ModelVector<RealPos>::getClassTypeSpec(), "The start times of the intervals (the first interval is implicit and starts at 0).", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        argument_rules.push_back( new ArgumentRule( "generationTime"    , RealPos::getClassTypeSpec(), "The generation time for the simulations.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        argument_rules.push_back( new ArgumentRule( "mutationRate"      , RealPos::getClassTypeSpec(), "The mutation rate.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        
        rules_set = true;
    }
    
    return argument_rules;
}


/** Get type spec */
const TypeSpec& CoalescentSFSSimulator::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get type spec */
void CoalescentSFSSimulator::printValue(std::ostream &o) const
{
    
    o << "CoalescentSFSSimulator";
}


/** Set a member variable */
void CoalescentSFSSimulator::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    
    if ( name == "generationTime")
    {
        generation_time = var;
    }
    else if ( name == "populationSizes")
    {
        population_sizes = var;
    }
    else if ( name == "changePoints")
    {
        change_points = var;
    }
    else if ( name == "mutationRate")
    {
        mutation_rate = var;
    }
    else
    {
        RevObject::setConstParameter(name, var);
    }
}
