### Version 1.2.0 Lagerstätte (unreleased)

#### New MCMC moves

 * slice sampling (mvSlice) can now do geometric scaling.

#### New models/analyses

 * New codon models:
   * Goldman-Yang (1994), Muse-Gaut (1994), FMutSel (Yang 2008)
   * Create of Codon models using stackable blocks fndNdS, fnX3, fnMutSel, fnMutSelAA
   * i.e. fnMutSel(F, fndNdS(omega, fnX3( fnGTR(er, pi) ) ) # GTR + X3 + dNdS + MutSel
 * New dinucleotide models: fnX2, fnMutSel
   * i.e. fnMutSel(F, fnX2( fnHKY(kappa, nuc_pi) ) )    # HKY + X2 + MutSel
  
#### Bug fixes

  * fixed occasional crashes when using recovering a tree from a checkpoint file due to rounding. Checkpointing now records doubles without rounding

#### New features

* optional weighting in fnAverageDistanceMatrix

#### Bug fixes

* fixed a bug when printing individual elements of average distance matrices and calculating their completeness



### Version 1.1.1

**Warning**: this version includes changes to the Random Number Generator, meaning analysis output will be different from v1.1.0, even when run with the same seed. 

#### New models/analyses

 * new PoMo rate matrices with selection

#### Bug fixes

 * fixed an error in the likelihood calculation for discrete characters when using ambiguous characters and partitioning standard data by state number
 * fixed a bug causing unbounded likelihoods when using PhyloBrownianProcessREML with unrooted trees
 * fixed a crash when using ordered rate matrices
 * fixed a crash when printing the value of a rate matrix
 * fixed a crash when using nested clade constraints
 * fixed a crash when using DEC models
 * fixed an issue with using simplex indexes in deterministic nodes
 * fixed a bug allowing NodeTimeSlideUniform to move sampled ancestor nodes


