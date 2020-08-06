#ifndef TimRateMatrixFunction_H
#define TimRateMatrixFunction_H

#include "TypedFunction.h"
#include "RateGenerator.h"

namespace RevBayesCore {
class DagNode;
class Simplex;
template <class valueType> class TypedDagNode;
    
    /**
     * @brief Tim rate matrix function.
     *
     * This function creates the Tim (transition model) rate matrix object by setting the exchangeability rates
     * and the base frequencies. The rate matrix takes care of the setting of the actual rates and transition probabilities.
     *
     * @param er The simplex of exchangeabilities (abccea): A<->C = G<->T, A<->G, A<->T = C<->G, C<->T
     * @param bf The simplex of base frequencies
     */
    class TimRateMatrixFunction : public TypedFunction<RateGenerator> {
        
    public:
        TimRateMatrixFunction(const TypedDagNode< Simplex > *er, const TypedDagNode< Simplex > *bf);
        virtual                                            ~TimRateMatrixFunction(void);                                                    //!< Virtual destructor
        
        // public member functions
        TimRateMatrixFunction*                              clone(void) const;                                                              //!< Create an independent clone
        void                                                update(void);
        
    protected:
        void                                                swapParameterInternal(const DagNode *oldP, const DagNode *newP);                        //!< Implementation of swaping parameters
        
    private:
        
        // members
        
        const TypedDagNode< Simplex >*                      exchangeability_rates; //!< A<->C = G<->T, A<->G, A<->T = C<->G, C<->T exchangeabilities
        const TypedDagNode< Simplex >*                      base_frequencies;
        
    };
    
}

#endif
