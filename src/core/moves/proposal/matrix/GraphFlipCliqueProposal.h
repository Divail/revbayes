#ifndef GraphFlipCliqueProposal_H
#define GraphFlipCliqueProposal_H

#include <set>
#include <string>

#include "Proposal.h"
#include "MatrixReal.h"
#include "StochasticNode.h"

namespace RevBayesCore {
    
    /**
     * The sliding operator.
     *
     * This move randomly picks an element of a matrix of positive real numbers.
     * That means, that we randomly pick the i-th row and j-th column with equal probability.
     * Then, we propose a sliding distance and slide the value.
     * The actual sliding distance is computed by delta = lambda * ( u - 0.5 )
     * where u ~ Uniform(0,1).
     * The proposal is thus m[i][j] += lambda * ( u - 0.5 )
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Nicolas Lartillot)
     * @since 2009-09-08, version 1.0
     *
     */
    class GraphFlipCliqueProposal : public Proposal {
        
    public:
        GraphFlipCliqueProposal( StochasticNode<MatrixReal> *n, const RbVector<long>& v, double l, double vp=0.1, double ep=1.0, bool s = false);                                                                      //!<  constructor
        
        // Basic utility functions
        void                                    cleanProposal(void);                                                                //!< Clean up proposal
        GraphFlipCliqueProposal*                clone(void) const;                                                                  //!< Clone object
        double                                  doProposal(void);                                                                   //!< Perform proposal
        const std::string&                      getProposalName(void) const;                                                        //!< Get the name of the proposal for summary printing
        void                                    printParameterSummary(std::ostream &o, bool name_only) const;                       //!< Print the parameter summary
        void                                    prepareProposal(void);                                                              //!< Prepare the proposal
        void                                    undoProposal(void);                                                                 //!< Reject the proposal
        
    protected:
        void                                    swapNodeInternal(DagNode *oldN, DagNode *newN);                                     //!< Swap the DAG nodes on which the Proposal is working on
        
    private:
        // parameters
        
        StochasticNode<RbVector<RbVector<double> > >* array;
        StochasticNode<MatrixReal>*                   matrix;
        
        double                                  set_sample_rate;
        double                                  edge_probability;
        double                                  vertex_probability;
        RbVector<long>                          vertices;
        size_t                                  vertex_list_length;
        double                                  storedValue;                                                                       //!< The value we propose.
        bool                                    symmetric;
        bool                                    undo_needed;
        
        struct Edge {
            Edge(size_t f, size_t t, double v) : from(f), to(t), value(v) {}
            size_t from;
            size_t to;
            double value;
        };
        std::vector<Edge>                       stored_edge_elements;
        
        
        
    };
    
}

#endif
