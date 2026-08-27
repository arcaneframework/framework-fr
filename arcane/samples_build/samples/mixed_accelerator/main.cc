// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2026 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------

#include <arcane/launcher/ArcaneLauncher.h>

#include "arcane/utils/NumArray.h"
#include "arcane/utils/Exception.h"
#include "arcane/utils/ITraceMng.h"
#include "arcane/utils/Math.h"
#include "arcane/utils/PlatformUtils.h"

#include "arcane/accelerator/core/IAcceleratorMng.h"

#include "arcane/accelerator/NumArrayViews.h"
#include "arcane/accelerator/RunQueue.h"
#include "arcane/accelerator/Runner.h"
#include "arcane/accelerator/RunCommandLoop.h"

// Pour permettre l'exécution avec un backend séquentiel et un accélérateur,
// vous devez lancer cet exemple comme ceci :
//
// ./MixedLauncher -A,UseAccelerator=1
//

using namespace Arcane;

/*
 * Applique l'EOS en utilisant le runner \a runner.
 *
 * Retourne le temps en microsecondes.
 */
Int64 _testRun(ITraceMng* tm,
               const Runner& runner,
               SmallSpan<const Real> in_pressure,
               SmallSpan<const Real> in_density,
               SmallSpan<const Real> in_adiabatic_cst,
               SmallSpan<Real> out_internal_energy,
               SmallSpan<Real> out_sound_speed,
               Int32 nb_loop)
{
  Int32 nb_value = in_pressure.size();
  tm->info() << "DoRun nb_value=" << nb_value << " policy=" << runner.executionPolicy();
  RunQueue queue(makeQueue(runner));
  Real begin_time = 0.0;
  // Parce que nous utilisons une mémoire unifiée, il peut y avoir un transfert de mémoire
  // pendant la première itération, nous ne mesurons donc pas ce temps.
  for (Int32 z = 0; z < (nb_loop + 1); ++z) {
    auto command = makeCommand(queue);
    command << RUNCOMMAND_LOOP1(index, nb_value)
    {
      Int32 i = index;
      Real pressure = in_pressure[i];
      Real adiabatic_cst = in_adiabatic_cst[i];
      Real density = in_density[i];
      out_internal_energy[i] = pressure / ((adiabatic_cst - 1.0) * density);
      out_sound_speed[i] = math::sqrt(adiabatic_cst * pressure / density);
    };
    if (z == 0)
      begin_time = Platform::getRealTime();
  }
  Real end_time = Platform::getRealTime();
  Real run_time = end_time - begin_time;
  tm->info() << "Time=" << run_time;
  return static_cast<Int64>(run_time * 1.0e6);
}

struct LoopTime
{
  Int32 nb_value = 0;
  Int64 accelerator_time = 0.0;
  Int64 host_sequential_time = 0.0;
};

void _testMixedLauncher()
{
  StandaloneAcceleratorMng launcher(ArcaneLauncher::createStandaloneAcceleratorMng());
  IAcceleratorMng* acc_mng = launcher.acceleratorMng();
  ITraceMng* tm = launcher.traceMng();
  Runner default_runner = acc_mng->runner();
  Runner sequential_runner(Accelerator::eExecutionPolicy::Sequential);

  Accelerator::eExecutionPolicy default_policy = default_runner.executionPolicy();
  Accelerator::eExecutionPolicy sequential_policy = sequential_runner.executionPolicy();
  UniqueArray<LoopTime> loop_times;
  // Applique l'EOS sur l'appareil et sur l'hôte pour plusieurs valeurs de 'nb_value'
  // et afficher lequel est le plus rapide
  for (Int32 nb_value = 100000; nb_value > 50; nb_value /= 2) {
    Arcane::NumArray<Real, MDDim1> pressure(nb_value);
    Arcane::NumArray<Real, MDDim1> density(nb_value);
    Arcane::NumArray<Real, MDDim1> adiabatic_cst(nb_value);
    Arcane::NumArray<Real, MDDim1> internal_energy(nb_value);
    Arcane::NumArray<Real, MDDim1> sound_speed(nb_value);
    for (int i = 0; i < nb_value; ++i) {
      pressure[i] = 0.125;
      adiabatic_cst[i] = 1.4;
      density[i] = 1.0 + static_cast<Real>(i) / static_cast<Real>(nb_value);
    }

    Int32 nb_loop = 1000;
    auto in_pressure = pressure.to1DSmallSpan();
    auto in_density = density.to1DSmallSpan();
    auto in_adiabatic_cst = adiabatic_cst.to1DSmallSpan();
    auto out_internal_energy = internal_energy.to1DSmallSpan();
    auto out_sound_speed = sound_speed.to1DSmallSpan();

    Int64 acc_time = _testRun(tm, default_runner,
                              in_pressure, in_density, in_adiabatic_cst,
                              out_internal_energy, out_sound_speed,
                              nb_loop);
    Int64 seq_time = _testRun(tm, sequential_runner,
                              in_pressure, in_density, in_adiabatic_cst,
                              out_internal_energy, out_sound_speed,
                              nb_loop);
    loop_times.add({ nb_value, acc_time, seq_time });
  }
  tm->info() << "Résultats (le temps est en microsecondes)";
  tm->info() << Trace::Width(12) << " NbValue "
             << " " << Trace::Width(12) << default_runner.executionPolicy()
             << " " << Trace::Width(12) << sequential_runner.executionPolicy()
             << " " << Trace::Width(12) << " Plus rapide"
             << " " << Trace::Width(12) << " TempsRelatif";
  for (const LoopTime& t : loop_times) {
    Real relative_time = static_cast<Real>(t.accelerator_time) / static_cast<Real>(t.host_sequential_time);
    Accelerator::eExecutionPolicy policy = (relative_time > 1.0) ? sequential_policy : default_policy;
    tm->info() << Trace::Width(12) << t.nb_value
               << " " << Trace::Width(12) << t.accelerator_time
               << " " << Trace::Width(12) << t.host_sequential_time
               << " " << Trace::Width(12) << policy
               << " " << Trace::Width(12) << Trace::Precision(5, relative_time);
  }
}

int main(int argc, char* argv[])
{
  auto func = [&] {
    Arcane::CommandLineArguments args(&argc, &argv);
    Arcane::ArcaneLauncher::init(args);
    _testMixedLauncher();
  };
  return Arcane::arcaneCallFunctionAndCatchException(func);
}
