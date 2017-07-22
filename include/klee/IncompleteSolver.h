//===-- IncompleteSolver.h --------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef KLEE_INCOMPLETESOLVER_H
#define KLEE_INCOMPLETESOLVER_H

#include "klee/Solver.h"
#include "klee/SolverImpl.h"

namespace klee {

/// IncompleteSolver - Base class for incomplete solver
/// implementations.
///
/// Incomplete solvers are useful for implementing optimizations which
/// may quickly compute an answer, but cannot always compute the
/// correct answer. They can be used with the StagedSolver to provide
/// a complete Solver implementation.
class IncompleteSolver {
public:
  /// PartialValidityMode - Represent a possibility incomplete query validity
  /// mode.
  enum PartialValidityMode {
    /// The query is true.
    MustBeTrue = 1,

    /// The query is false.
    MustBeFalse = -1,

    /// The query is not false (a true assignment is known to exist).
    MayBeTrue = 2,

    /// The query is not true (a false assignment is known to exist).
    MayBeFalse = -2,

    /// The query is known to have both true and false assignments.
    TrueOrFalse = 0,

    /// The validity of the query is unknown.
    None = 3
  };

  static PartialValidityMode negatePartialValidityMode(PartialValidityMode pv);

public:
  IncompleteSolver() {}
  virtual ~IncompleteSolver() {}

  /// computeValidityMode - Compute a partial validity mode for the given query.
  ///
  /// The passed query expression should be non-constant and have bool type.
  ///
  /// The IncompleteSolver class provides an implementation of
  /// computeValidityMode using computeTruth. Sub-classes may override this if a
  /// more efficient implementation is available.
  ///
  /// \return IncompleteSolver::MustBeTrue if provably
  /// \f[ \forall X constraints(X) \to query(X) \f]
  /// else IncompleteSolver::MustBeFalse if provably
  /// \f[ \forall X constraints(X) \to \lnot query(X) \f]
  /// else IncompleteSolver::MayBeTrue if provably
  /// \f[ \exists X constraints(X) \wedge query(X) \f]
  /// else IncompleteSolver::MayBeFalse if provably
  /// \f[ \exists X constraints(X) \wedge \lnot query(X) \f]
  /// else IncompleteSolver::TrueOrFalse if provably both
  /// \f[ \exists X constraints(X) \wedge query(X) \f]
  /// and
  /// \f[ \exists X constraints(X) \wedge \lnot query(X) \f]
  /// else IncompleteSolver::None
  virtual IncompleteSolver::PartialValidityMode
  computeValidityMode(const Query &);

  /// computeTruth - Determine whether the given query expression is true or not
  /// (may be false) given the constraints.
  ///
  /// The passed query expression should be non-constant and have bool type.
  ///
  /// This method should evaluate the logical formula:
  ///
  /// \f[ \forall X constraints(X) \to query(X) \f]
  ///
  /// Where \f$X\f$ is some assignment, \f$constraints(X)\f$ are the constraints
  /// in the query and \f$query(X)\f$ is the query expression.
  ///
  /// \return IncompleteSolver::MustBeTrue if the above formula is provably
  /// true, otherwise IncompleteSolver::MayBeFalse if provably
  /// \f[ \exists X constraints(X) \wedge \lnot query(X) \f]
  virtual IncompleteSolver::PartialValidityMode computeTruth(const Query &) = 0;

  /// computeValue - Attempt to compute a value for the given expression.
  virtual bool computeValue(const Query&, ref<Expr> &result) = 0;

  /// computeInitialValues - Attempt to compute the constant values
  /// for the initial state of each given object. If a correct result
  /// is not found, then the values array must be unmodified.
  virtual bool computeInitialValues(const Query&,
                                    const std::vector<const Array*> 
                                      &objects,
                                    std::vector< std::vector<unsigned char> > 
                                      &values,
                                    bool &hasSolution) = 0;
};

/// StagedSolver - Adapter class for staging an incomplete solver with
/// a complete secondary solver, to form an (optimized) complete
/// solver.
class StagedSolverImpl : public SolverImpl {
private:
  IncompleteSolver *primary;
  Solver *secondary;
  
public:
  StagedSolverImpl(IncompleteSolver *_primary, Solver *_secondary);
  ~StagedSolverImpl();
    
  bool computeTruth(const Query&, bool &isValid);
  bool computeValidityMode(const Query &, Solver::ValidityMode &result);
  bool computeValue(const Query&, ref<Expr> &result);
  bool computeInitialValues(const Query&,
                            const std::vector<const Array*> &objects,
                            std::vector< std::vector<unsigned char> > &values,
                            bool &hasSolution);
  SolverRunStatus getOperationStatusCode();
  char *getConstraintLog(const Query&);
  void setCoreSolverTimeout(double timeout);
};

}

#endif
