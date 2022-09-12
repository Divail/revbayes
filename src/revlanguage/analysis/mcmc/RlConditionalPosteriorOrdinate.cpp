#include "RlConditionalPosteriorOrdinate.h"

#include <ostream>
#include <string>
#include <vector>

#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "Delimiter.h"
#include "RevObject.h"
#include "Real.h"
#include "RlString.h"
#include "TypeSpec.h"
#include "MemberProcedure.h"
#include "MethodTable.h"
#include "RevPtr.h"
#include "RevVariable.h"
#include "ConditionalPosteriorOrdinate.h"
#include "WorkspaceToCoreWrapperObject.h"

namespace RevLanguage { class Argument; }


using namespace RevLanguage;

ConditionalPosteriorOrdinate::ConditionalPosteriorOrdinate() : WorkspaceToCoreWrapperObject<RevBayesCore::ConditionalPosteriorOrdinate>()
{

    ArgumentRules* marginalArgRules = new ArgumentRules();
    methods.addFunction(new MemberProcedure( "marginal", Real::getClassTypeSpec(), marginalArgRules) );

}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
ConditionalPosteriorOrdinate* ConditionalPosteriorOrdinate::clone(void) const
{
    
    return new ConditionalPosteriorOrdinate(*this);
}


void ConditionalPosteriorOrdinate::constructInternalObject( void )
{
    // we free the memory first
    delete value;
    
    // get the parameter values
    const std::string&   fn      = static_cast<const RlString &>( filename->getRevObject() ).getValue();
    const std::string&   pn      = static_cast<const RlString &>( powerColumnName->getRevObject() ).getValue();
    const std::string&   ln      = static_cast<const RlString &>( likelihoodColumnName->getRevObject() ).getValue();
    const std::string&   del     = static_cast<const RlString &>( delimiter->getRevObject() ).getValue();
    
    value = new RevBayesCore::ConditionalPosteriorOrdinate(fn, pn, ln, del);
    
}


/* Map calls to member methods */
RevPtr<RevVariable> ConditionalPosteriorOrdinate::executeMethod(std::string const &name, const std::vector<Argument> &args, bool &found)
{
    
    if (name == "marginal")
    {
        found = true;
        
        double ml = value->marginalLikelihood();
        
        return new RevVariable( new Real( ml ) );
    }
    
    return RevObject::executeMethod( name, args, found );
}


/** Get Rev type of object */
const std::string& ConditionalPosteriorOrdinate::getClassType(void)
{
    
    static std::string rev_type = "ConditionalPosteriorOrdinate";
    
    return rev_type;
}

/** Get class type spec describing type of object */
const TypeSpec& ConditionalPosteriorOrdinate::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( WorkspaceToCoreWrapperObject<RevBayesCore::ConditionalPosteriorOrdinate>::getClassTypeSpec() ) );
    
    return rev_type_spec;
}


/**
 * Get the Rev name for the constructor function.
 *
 * \return Rev name of constructor function.
 */
std::string ConditionalPosteriorOrdinate::getConstructorFunctionName( void ) const
{
    // create a constructor function name variable that is the same for all instance of this class
    std::string c_name = "steppingStoneSampler";
    
    return c_name;
}


/** Return member rules (no members) */
const MemberRules& ConditionalPosteriorOrdinate::getParameterRules(void) const
{
    
    static MemberRules samplerMemberRules;
    static bool rules_set = false;
    
    if ( !rules_set )
    {
        samplerMemberRules.push_back( new ArgumentRule("filename"            , RlString::getClassTypeSpec(), "The name of the file where the likelhood samples are stored.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        samplerMemberRules.push_back( new ArgumentRule("powerColumnName"     , RlString::getClassTypeSpec(), "The name of the column of the powers.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        samplerMemberRules.push_back( new ArgumentRule("likelihoodColumnName", RlString::getClassTypeSpec(), "The name of the column of the likelihood samples.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        samplerMemberRules.push_back( new Delimiter() );

        rules_set = true;
    }
    
    return samplerMemberRules;
}

/** Get type spec */
const TypeSpec& ConditionalPosteriorOrdinate::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get type spec */
void ConditionalPosteriorOrdinate::printValue(std::ostream &o) const
{
    
    o << "ConditionalPosteriorOrdinate";
}


/** Set a member variable */
void ConditionalPosteriorOrdinate::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    
    if ( name == "likelihoodColumnName")
    {
        likelihoodColumnName = var;
    }
    else if ( name == "powerColumnName")
    {
        powerColumnName = var;
    }
    else if ( name == "filename")
    {
        filename = var;
    }
    else if ( name == "delimiter" || name == "separator" )
    {
        delimiter = var;
    }
    else
    {
        RevObject::setConstParameter(name, var);
    }
}
