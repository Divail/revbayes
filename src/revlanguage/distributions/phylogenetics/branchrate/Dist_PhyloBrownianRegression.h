#ifndef Dist_PhyloBrownianRegression_H
#define Dist_PhyloBrownianRegression_H

#include <math.h>
#include <iosfwd>
#include <string>
#include <vector>

#include "ContinuousCharacterData.h"
#include "RlContinuousCharacterData.h"
#include "RlTypedDistribution.h"
#include "DagMemberFunction.h"
#include "DeterministicNode.h"
#include "DynamicNode.h"
#include "RevObject.h"
#include "RevPtr.h"
#include "RevVariable.h"
#include "RlDagMemberFunction.h"
#include "RlDeterministicNode.h"
#include "RlStochasticNode.h"
#include "RlTypedFunction.h"
#include "StochasticNode.h"
#include "TypedDagNode.h"
#include "TypedDistribution.h"
#include "TypedFunction.h"

namespace RevLanguage {
class TypeSpec;
    
    class Dist_PhyloBrownianRegression :  public TypedDistribution< ContinuousCharacterData > {
        
    public:
        Dist_PhyloBrownianRegression( void );
        virtual ~Dist_PhyloBrownianRegression();
        
        // Basic utility functions
        Dist_PhyloBrownianRegression*                   clone(void) const;                                                                      //!< Clone the object
        static const std::string&                       getClassType(void);                                                                     //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                                                 //!< Get class type spec
        std::string                                     getDistributionFunctionName(void) const;                                                //!< Get the Rev-name for this distribution.
        const TypeSpec&                                 getTypeSpec(void) const;                                                                //!< Get the type spec of the instance
        const MemberRules&                              getParameterRules(void) const;                                                          //!< Get member rules (const)
        void                                            printValue(std::ostream& o) const;                                                      //!< Print the general information on the function ('usage')
        
        
        // Distribution functions you have to override
        RevBayesCore::TypedDistribution< RevBayesCore::ContinuousCharacterData >*      createDistribution(void) const;
        
    protected:
        
        void                                            setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);       //!< Set member variable
        
        
    private:
        
        RevPtr<const RevVariable>                       tree;
        RevPtr<const RevVariable>                       branch_rates;
        RevPtr<const RevVariable>                       means;
        RevPtr<const RevVariable>                       slopes;
        RevPtr<const RevVariable>                       predictors;
        
        
    };
    
}

#endif

