#ifndef NodeTimeSlideUniformAgeConstrainedProposal_H
#define NodeTimeSlideUniformAgeConstrainedProposal_H

#include <string>

#include "Proposal.h"
#include "StochasticNode.h"
#include "Tree.h"
#include "Clade.h"

namespace RevBayesCore {
    
    /**
     * The node-age slide proposal operator using a Uniform distribution.
     *
     * This node-age proposal is a Uniform-sliding proposal on rooted subtrees without changing the topology.
     * That is, we pick a random node which is not the root.
     * Then, we pick an age between the parent and the oldest sampled descendant drawn from a Uniform distribution centered around the current age.
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2012-07-12, version 1.0
     *
     */
    class NodeTimeSlideUniformAgeConstrainedProposal : public Proposal {
        
    public:
        NodeTimeSlideUniformAgeConstrainedProposal( StochasticNode<Tree> *n, const RbVector<Clade> &c);                 //!<  constructor
        
        // Basic utility functions
        void                                                cleanProposal(void);                                        //!< Clean up proposal
        NodeTimeSlideUniformAgeConstrainedProposal*         clone(void) const;                                          //!< Clone object
        double                                              doProposal(void);                                           //!< Perform proposal
        const std::string&                                  getProposalName(void) const;                                //!< Get the name of the proposal for summary printing
        void                                                prepareProposal(void);                                      //!< Prepare the proposal
        void                                                printParameterSummary(std::ostream &o, bool name_only) const;               //!< Print the parameter summary
        void                                                undoProposal(void);                                         //!< Reject the proposal
        
    protected:
        
        void                                                swapNodeInternal(DagNode *oldN, DagNode *newN);             //!< Swap the DAG nodes on which the Proposal is working on
        
        
    private:
        
        // parameters
        StochasticNode<Tree>*                               variable;                                                   //!< The variable the Proposal is working on
        
        // stored objects to undo proposal
        RbVector<Clade>                                     constraints;
//        TopologyNode*                                       stored_node;
        double                                              stored_age;
        
    };
    
}

#endif

