#ifndef Func_PoMo2N_H
#define Func_PoMo2N_H

#include "RlRateMatrix.h"
#include "RlTypedFunction.h"

#include <map>
#include <string>

namespace RevLanguage {
    
    /**
     * The RevLanguage wrapper of the PoMo2N rate matrix function.
     *
     * The RevLanguage wrapper of the PoMo2N rate matrix connects
     * the variables/parameters of the function and creates the internal HkyRateMatrixFunction object.
     * Please read the PoMo2NRateMatrixFunction.h for more info.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2014-08-14, version 1.0
     *
     */
    class Func_PoMo2N : public TypedFunction<RateMatrix> {
        
    public:
        Func_PoMo2N( void );
        
        // Basic utility functions
        Func_PoMo2N*                                                        clone(void) const;                                          //!< Clone the object
        static const std::string&                                           getClassType(void);                                         //!< Get Rev type
        static const TypeSpec&                                              getClassTypeSpec(void);                                     //!< Get class type spec
        std::string                                                         getFunctionName(void) const;                                //!< Get the primary name of the function in Rev
        const TypeSpec&                                                     getTypeSpec(void) const;                                    //!< Get the type spec of the instance
        
        // Function functions you have to override
        RevBayesCore::TypedFunction< RevBayesCore::RateGenerator >*         createFunction(void) const;                                 //!< Create a function object
        const ArgumentRules&                                                getArgumentRules(void) const;                               //!< Get argument rules
        
    };
    
}

#endif


