//==============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  This software developed by Applied Research Laboratories at the University of Texas at Austin.
//  Copyright 2004-2020, The Board of Regents of The University of Texas System
//
//==============================================================================

//==============================================================================
//
//  This software developed by Applied Research Laboratories at the University of
//  Texas at Austin, under contract to an agency or agencies within the U.S. 
//  Department of Defense. The U.S. Government retains all rights to use,
//  duplicate, distribute, disclose, or release this software. 
//
//  Pursuant to DoD Directive 523024 
//
//  DISTRIBUTION STATEMENT A: This software has been approved for public 
//                            release, distribution is unlimited.
//
//==============================================================================

/** @file SP3EphemerisStore.hpp
 * Store a tabular list of position and clock bias (perhaps also
 * velocity and clock drift) data from SP3 file(s) for several
 * satellites; access the tables to compute values at any timetag,
 * within the limits of the data, from this table via interpolation.
 * An option allows assigning the clock store to RINEX clock files,
 * with separate timestep and interpolation algorithm. */

#ifndef GPSTK_SP3_EPHEMERIS_STORE_INCLUDE
#define GPSTK_SP3_EPHEMERIS_STORE_INCLUDE

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

#include "Exception.hpp"
#include "SatID.hpp"
#include "CommonTime.hpp"
#include "XvtStore.hpp"

#include "FileStore.hpp"
#include "ClockSatStore.hpp"
#include "PositionSatStore.hpp"

#include "SP3Header.hpp"
#include "Rinex3ClockHeader.hpp"

namespace gpstk
{
      /// @ingroup GNSSEph
      //@{

      /** Store position and clock bias (and perhaps velocity and
       * drift) data from SP3 files, using (separate) stores based on
       * TabularSatStore. An option allows the clock store to be taken
       * from RINEX clock files instead.  Keep a FileStore for the SP3
       * input files, and for the RINEX clock files, if they are
       * used. Inherit XvtStore for the interface it defines. */
   class SP3EphemerisStore : public XvtStore<SatID>
   {

         // member data
   private:
         /** Time system for this store. Must set, and keep
          * consistent, in loadFile.  Returned in getTimeSystem()
          * (virtual in XvtStore).
          * @note usually GPS, but CANNOT assume so. */
      TimeSystem storeTimeSystem;

         /// PositionSatStore for SP3 ephemeris data
      PositionSatStore posStore;

         /// ClockSatStore for SP3 OR RINEX clock data
      ClockSatStore clkStore;

         /// FileStore for the SP3 input files
      FileStore<SP3Header> SP3Files;

         /// FileStore for the (optional) RINEX clock input files
      FileStore<Rinex3ClockHeader> clkFiles;

         /** flag indicating whether the clock store contains data
          * from SP3 (true, the default) or Rinex clock (false)
          * files */
      bool useSP3clock;

         /** Flag to reject all data when there are bad positions,
          * default true. */
      bool rejectBadPosFlag;

         /** Flag to reject all data when there are bad clocks,
          * default true.
          * @note this flag has no effect if the clock store comes
          * from RINEX clock files. */
      bool rejectBadClockFlag;

         /** Flag to reject predicted position data, using
          * orbitPredFlag in SP3Data. */
      bool rejectPredPosFlag;

         /** Flag to reject predicted clock data, using clockPredFlag
          * in SP3Data.
          * @note this flag has no effect if the clock store comes
          * from RINEX clock files. */
      bool rejectPredClockFlag;

         // member functions

         /** Private utility routine used by the loadFile and
         * loadSP3File routines.  Store position (velocity) and clock
         * data from SP3 files in clock and position stores. Also
         * update the FileStore with the filename and SP3 header.
         * Check time systems consistentcy, and if possible set store
         * time system.
         * @throw Exception
         */
      void loadSP3Store(const std::string& filename, bool fillClockStore);

   public:

         /// Default constructor
      SP3EphemerisStore() throw() : storeTimeSystem(TimeSystem::Any),
         useSP3clock(true),
         rejectBadPosFlag(true),
         rejectBadClockFlag(true),
         rejectPredPosFlag(false),
         rejectPredClockFlag(false)
      { }

         /// Destructor
      virtual ~SP3EphemerisStore()
      { }

         // XvtStore interface:
         /** Returns the position, velocity, and clock offset of the
          * indicated object in ECEF coordinates (meters) at the
          * indicated time.
          * @param[in] sat the satellite of interest
          * @param[in] ttag the time to look up
          * @return the Xvt of the object at the indicated time
          * @throw InvalidRequest If the request can not be completed for any
          *    reason, this is thrown. The text may have additional
          *    information as to why the request failed. */
      virtual Xvt getXvt(const SatID& sat, const CommonTime& ttag) const;

         /** Compute the position, velocity and clock offset of the
          * indicated object in ECEF coordinates (meters) at the
          * indicated time.
          * This method functions similarly to getXvt() except that it
          * does not throw an exception for any reason.  Instead, the
          * caller is expected to check the value of the "health"
          * field of the returned Xvt and decide what to do with the
          * data.
          * @note This function ignores the onlyHealthy flag as health
          *   information is not available in the SP3 format.
          * @note The health flag in the returned Xvt can have one of
          *   two values, "Unavailable", in which case the Xvt could
          *   not be determined from the data in the store, or
          *   "Unused" in which case the Xvt could be determined.
          *   "Unused" because the SP3 format does not include health
          *   status information.
          * @param[in] id the object's identifier
          * @param[in] t the time to look up
          * @return the Xvt of the object at the indicated time */
      virtual Xvt computeXvt(const SatID& id, const CommonTime& t) const
         throw();

         /** Get the satellite health at a specific time.
          * @param[in] id the object's identifier
          * @param[in] t the time to look up
          * @return "Unused" at all times as the SP3 format does not
          *   provide health status. */
      virtual Xvt::HealthStatus getSVHealth(const SatID& id,
                                            const CommonTime& t) const throw();

         /** Dump information about the store to an ostream.
          * @param[in] os ostream to receive the output; defaults to std::cout
          * @param[in] detail integer level of detail to provide;
          *   allowed values are
          *    0: number of satellites, time step and time limits
          *    1: above plus flags, gap and interval values, and
          *       number of data/sat
          *    2: above plus all the data tables */
      virtual void dump(std::ostream& os = std::cout, short detail = 0)
         const throw()
      {
            // may have to re-write this...
         os << "Dump SP3EphemerisStore:" << std::endl;
            // 'reject' flags
         os << (rejectBadPosFlag ? " Reject":" Do not reject")
            << " bad positions." << std::endl;
         os << (rejectBadClockFlag ? " Reject":" Do not reject")
            << " bad clocks." << std::endl;
         os << (rejectPredPosFlag ? " Reject":" Do not reject")
            << " predicted positions." << std::endl;
         os << (rejectPredClockFlag ? " Reject":" Do not reject")
            << " predicted clocks." << std::endl;

         SP3Files.dump(os, detail);
         posStore.dump(os, detail);
         if(!useSP3clock) clkFiles.dump(os, detail);
         clkStore.dump(os, detail);

         os << "End dump SP3EphemerisStore." << std::endl;
      }

         /** Edit the dataset, removing data outside the indicated
          * time interval
          * @param[in] tmin defines the beginning of the time interval
          * @param[in] tmax defines the end of the time interval */
      virtual void edit(const CommonTime& tmin, 
                        const CommonTime& tmax = CommonTime::END_OF_TIME) throw()
      {
         posStore.edit(tmin, tmax);
         clkStore.edit(tmin, tmax);
      }

         /// Clear the dataset, meaning remove all data
      virtual void clear(void) throw()
      { clearPosition(); clearClock(); }
 
         /// Return time system (@note usually GPS, but CANNOT assume so)
      virtual TimeSystem getTimeSystem(void) const throw()
      { return storeTimeSystem; }

         /** Determine the earliest time for which this object can
          * successfully determine the Xvt for any object.
          * @return the earliest time in the table
          * @throw InvalidRequest if the object has no data. */
      virtual CommonTime getInitialTime() const;

         /** Determine the latest time for which this object can
          * successfully determine the Xvt for any object.
          * @return the latest time in the table
          * @throw InvalidRequest if the object has no data. */
      virtual CommonTime getFinalTime() const;

         /// Return true if IndexType=SatID is present in the data tables
      virtual bool isPresent(const SatID& sat) const throw()
      { return (posStore.isPresent(sat) && clkStore.isPresent(sat)); }

         /// Return true if velocity is present in the data tables
      virtual bool hasVelocity() const throw()
      {  return posStore.hasVelocity(); }

         // end of XvtStore interface

         /** Dump information about the position store to an ostream.
          * @param[in] os ostream to receive the output; defaults to std::cout
          * @param[in] detail integer level of detail to provide;
          *   allowed values are
          *    0: number of satellites, time step and time limits
          *    1: above plus flags, gap and interval values, and
          *       number of data/sat
          *    2: above plus all the data tables */
      void dumpPosition(std::ostream& os = std::cout, short detail = 0)
         const throw()
      {
         SP3Files.dump(os, detail);
         posStore.dump(os, detail);
      }

         /** Dump information about the clock store to an ostream.
          * @param[in] os ostream to receive the output; defaults to std::cout
          * @param[in] detail integer level of detail to provide;
          *   allowed values are
          *    0: number of satellites, time step and time limits
          *    1: above plus flags, gap and interval values, and
          *       number of data/sat
          *    2: above plus all the data tables */
      void dumpClock(std::ostream& os = std::cout, short detail = 0)
         const throw()
      {
         if(useSP3clock)
            SP3Files.dump(os, detail);
         else
            clkFiles.dump(os, detail);
         clkStore.dump(os, detail);
      }

         /** Return the position for the given satellite at the given time.
          * @param[in] sat the SatID of the satellite of interest
          * @param[in] ttag the time (CommonTime) of interest
          * @return Triple containing the position ECEF XYZ meters
          * @throw InvalidRequest if result cannot be computed, for
          *   example because:
          *  a) the time t does not lie within the time limits of the
          *     data table
          *  b) checkDataGap is true and there is a data gap
          *  c) checkInterval is true and the interval is larger than
          *     maxInterval */
      Triple getPosition(const SatID sat, const CommonTime ttag)
         const;

         /** Return the velocity for the given satellite at the given time
          * @param[in] sat the SatID of the satellite of interest
          * @param[in] ttag the time (CommonTime) of interest
          * @return Triple containing the velocity ECEF XYZ meters/second
          * @throw InvalidRequest if result cannot be computed, for
          *   example because
          *  a) the time t does not lie within the time limits of the
          *     data table
          *  b) checkDataGap is true and there is a data gap
          *  c) checkInterval is true and the interval is larger than
          *     maxInterval */
      Triple getVelocity(const SatID sat, const CommonTime ttag)
         const;

         /** Return the acceleration for the given satellite at the given time
          * @param[in] sat the SatID of the satellite of interest
          * @param[in] ttag the time (CommonTime) of interest
          * @return Triple containing the acceleration ECEF XYZ
          *   meters/second/second
          * @throw InvalidRequest if result cannot be computed, for
          *   example because
          *  a) the time t does not lie within the time limits of the
          *     data table
          *  b) checkDataGap is true and there is a data gap
          *  c) checkInterval is true and the interval is larger than
          *     maxInterval */
      Triple getAcceleration(const SatID sat, const CommonTime ttag)
         const
      { return posStore.getAcceleration(sat,ttag); }


         /** Clear the position dataset only, meaning remove all data
          * from the tables. */
      virtual void clearPosition(void) throw()
      { posStore.clear(); }

         /** Clear the clock dataset only, meaning remove all data
          * from the tables. */
      virtual void clearClock(void) throw()
      { clkStore.clear(); }

   
         /** Choose to load the clock data tables from RINEX clock
          * files. This will clear the clock store; loadFile() or
          * loadRinexClockFile() should be called after this, to load
          * data into the clock store from RINEX clock files.  This
          * routine has no effect if the clock store is already set to
          * RINEX clock.
          * @note will be called by loadRinexClockFile() if clock
          * store is set to SP3. */
      void useRinexClockData(void) throw()
      {
         if(!useSP3clock) return;
         useSP3clock = false;
         clearClock();
      }

         /** Choose to load the clock data tables from SP3 files (this
         * is the default).  This will clear the clock store; if the
         * position store has already been loaded it should also be
         * cleared. The routines loadFile() or loadSP3File() should be
         * called after this, to load data into the clock store from
         * SP3 files.
         * @note this will also load position data into the position
         * store.  This routine has no effect if the clock store is
         * already set to SP3. */
      void useSP3ClockData(void) throw()
      {
         if(useSP3clock) return;
         useSP3clock = true;
         clearClock();
      }

         /** Get the earliest time of data in the position store.
          * @return CommonTime the first time
          * @throw InvalidRequest if there is no data */
      CommonTime getPositionInitialTime(void) const
      { return posStore.getInitialTime(); }

         /** Get the latest time of data in the position store.
          * @return CommonTime the latest time
          * @throw InvalidRequest if there is no data */
      CommonTime getPositionFinalTime(void) const
      { return posStore.getFinalTime(); }

         /** Get the earliest time of data in the clock store.
          * @return CommonTime the first time
          * @throw InvalidRequest if there is no data */
      CommonTime getClockInitialTime(void) const
      { return clkStore.getInitialTime(); }

         /** Get the latest time of data in the clock store.
          * @return CommonTime the latest time
          * @throw InvalidRequest if there is no data */
      CommonTime getClockFinalTime(void) const
      { return clkStore.getFinalTime(); }

         /** Get the earliest time of data in the position store for
          * the given satellite.
          * @return CommonTime the first time
          * @throw InvalidRequest if there is no data */
      CommonTime getPositionInitialTime(const SatID& sat) const
      { return posStore.getInitialTime(sat); }

         /** Get the latest time of data in the position store for the
          * given satellite.
          * @return CommonTime the latest time
          * @throw InvalidRequest if there is no data */
      CommonTime getPositionFinalTime(const SatID& sat) const
      { return posStore.getFinalTime(sat); }

         /** Get the earliest time of data in the clock store for the
          * given satellite.
          * @return CommonTime the first time
          * @throw InvalidRequest if there is no data */
      CommonTime getClockInitialTime(const SatID& sat) const
      { return clkStore.getInitialTime(sat); }

         /** Get the latest time of data in the clock store for the
          * given satellite.
          * @return CommonTime the latest time
          * @throw InvalidRequest if there is no data */
      CommonTime getClockFinalTime(const SatID& sat) const
      { return clkStore.getFinalTime(sat); }

         /** Get the earliest time of both clock and position data in
         * the store for the given satellite.
         * @return CommonTime the first time
         * @throw InvalidRequest if there is no data */
      CommonTime getInitialTime(const SatID& sat) const;

         /** Get the latest time of both clock and position data in
          * the store for the given satellite.
          * @return the latest time
          * @throw InvalidRequest if there is no data */
      CommonTime getFinalTime(const SatID& sat) const;


         /** Get the nominal time step in seconds for the position
          * data and the given sat */
      double getPositionTimeStep(const SatID& sat) const throw()
      { return posStore.nomTimeStep(sat); }

         /** Get the nominal time step in seconds for the clock data
          * and the given sat */
      double getClockTimeStep(const SatID& sat) const throw()
      { return clkStore.nomTimeStep(sat); }


         /// Get current interpolation order for the position table
      unsigned int getPositionInterpOrder(void) const throw()
      { return posStore.getInterpolationOrder(); }

         /** Set the interpolation order for the position table; it is
          * forced to be even. */
      void setPositionInterpOrder(unsigned int order) throw()
      { posStore.setInterpolationOrder(order); }

         /** Get current interpolation order for the clock data
          * (meaningless if the interpolation type is linear). */
      unsigned int getClockInterpOrder(void) throw()
      { return clkStore.getInterpolationOrder(); }

         /** Set the interpolation order for the clock table; it is
          * forced to be even.  This is ignored if the clock
          * interpolation type is linear. */
      void setClockInterpOrder(unsigned int order) throw()
      { clkStore.setInterpolationOrder(order); }

         /** Set the type of clock interpolation to Lagrange (the
          * default); set the order of the interpolation with
          * setClockInterpolationOrder(order); */
      void setClockLagrangeInterp(void) throw()
      { clkStore.setLagrangeInterp(); }

         /** Set the type of clock interpolation to linear
          * (interpolation order is ignored). */
      void setClockLinearInterp(void) throw()
      { clkStore.setLinearInterp(); }


         /** Get a list (std::vector) of SatIDs present in both clock
          * and position stores */
      std::vector<SatID> getSatList(void) const throw()
      {
         std::vector<SatID> posList(posStore.getSatList());
         std::vector<SatID> clkList(clkStore.getSatList());
         std::vector<SatID> retList;
         for(size_t i=0; i<posList.size(); i++)
            if(std::find(clkList.begin(),clkList.end(),posList[i]) != clkList.end())
               retList.push_back(posList[i]);
         return retList;
      }

         // Get a set of the SatIDs present in both clock and position stores
      std::set<SatID> getIndexSet(void) const
      {
         std::set<SatID> retSet;
         try
         {
            std::vector<SatID> posList(posStore.getSatList());
            std::vector<SatID> clkList(clkStore.getSatList());
            for(size_t i=0; i<posList.size(); i++)
               if(std::find(clkList.begin(),clkList.end(),posList[i]) != clkList.end())
                  retSet.insert(posList[i]);
         }
         catch(gpstk::Exception)
         {
            // do nothing
         }
         return retSet;
      }

         /// Get a list (std::vector) of SatIDs present in the position store
      std::vector<SatID> getPositionSatList(void) const throw()
      { return posStore.getSatList(); }

         /// Get a list (std::vector) of SatIDs present in the clock store
      std::vector<SatID> getClockSatList(void) const throw()
      { return clkStore.getSatList(); }


         /// Get the total number of (position) data records in the store
      inline int ndata(void) const throw()
      { return posStore.ndata(); }

         /// Get the number of (position) data records for the given sat
      inline int ndata(const SatID& sat) const throw()
      { return posStore.ndata(sat); }

         /** Get the number of (position) data records for the given
          * satellite system */
      inline int ndata(const SatelliteSystem& sys) const throw()
      { return posStore.ndata(sys); }

         /// Get the total number of position data records in the store
      inline int ndataPosition(void) const throw()
      { return posStore.ndata(); }

         /// Get the number of position data records for the given sat
      inline int ndataPosition(const SatID& sat) const throw()
      { return posStore.ndata(sat); }

         /** Get the number of position data records for the given
          * satellite system */
      inline int ndataPosition(const SatelliteSystem& sys) const throw()
      { return posStore.ndata(sys); }

         /// Get the total number of clock data records in the store
      inline int ndataClock(void) const throw()
      { return clkStore.ndata(); }

         /// Get the number of clock data records for the given sat
      inline int ndataClock(const SatID& sat) const throw()
      { return clkStore.ndata(sat); }

         /** Get the number of clock data records for the given
          * satellite system */
      inline int ndataClock(const SatelliteSystem& sys) const throw()
      { return clkStore.ndata(sys); }

         /// same as ndataPosition()
      inline int size(void) const throw() { return ndataPosition(); }


         /** Load an SP3 ephemeris file; if the clock store uses RINEX
          * clock files, this routine will also accept that file type
          * and load the data into the clock store. This routine will
          * may set the velocity, acceleration, bias or drift 'have'
          * flags.
          * @param filename name of file (SP3 or RINEX clock format) to load 
          * @throw Exception if time step is inconsistent with previous value
          */
      void loadFile(const std::string& filename);

         /** Load an SP3 ephemeris file; may set the velocity and
          * acceleration flags.  If the clock store uses RINEX clock
          * data, this will ignore the clock data.
          * @param filename name of file (SP3 format) to load 
          * @throw Exception if time step is inconsistent with previous value
          */
      void loadSP3File(const std::string& filename);

         /** Load a RINEX clock file; may set the 'have' bias and
          * drift flags.  If clock store is set to use SP3 data, this
          * will call useRinexClockData()
          * @param filename name of file (RINEX clock format) to load 
          * @throw Exception if time step is inconsistent with previous value
          */
      void loadRinexClockFile(const std::string& filename);


         /** Add a complete PositionRecord to the store; this is the
          * preferred method of adding data to the tables.
          * @note If these addXXX() routines are used more than once
          * for the same record (sat,ttag), be aware that since ttag
          * is used as they key in a std::map, the value used must be
          * EXACTLY the same in all calls; (numerical noise could
          * cause the std::map to consider two "equal" ttags as
          * different).
          * @throw InvalidRequest
          */
      void addPositionRecord(const SatID& sat, const CommonTime& ttag,
                             const PositionRecord& data)
      {
         try { posStore.addPositionRecord(sat,ttag,data); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }

         /** Add position data to the store
          * @throw InvalidRequest
          */
      void addPositionData(const SatID& sat, const CommonTime& ttag,
                           const Triple& Pos, const Triple& sig)
      {
         try { posStore.addPositionData(sat,ttag,Pos,sig); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }

         /** Add velocity data to the store
          * @throw InvalidRequest
          */
      void addVelocityData(const SatID& sat, const CommonTime& ttag,
                           const Triple& Vel, const Triple& sig)
      {
         try { posStore.addVelocityData(sat,ttag,Vel,sig); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }

         /** Add a complete ClockRecord to the store; this is the
          * preferred method of adding data to the tables.
          * @note If these addXXX() routines are used more than once
          * for the same record (sat,ttag), be aware that since ttag
          * is used as they key in a std::map, the value used must be
          * EXACTLY the same in all calls; (numerical noise could
          * cause the std::map to consider two "equal" ttags as
          * different).
          * @throw InvalidRequest
          */
      void addClockRecord(const SatID& sat, const CommonTime& ttag,
                          const ClockRecord& rec)
      {
         try { clkStore.addClockRecord(sat,ttag,rec); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }

         /** Add clock bias data (only) to the store
          * @throw InvalidRequest
          */
      void addClockBias(const SatID& sat, const CommonTime& ttag,
                        const double& bias, const double& sig=0.0)
      {
         try { clkStore.addClockBias(sat,ttag,bias,sig); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }

         /** Add clock drift data (only) to the store
          * @throw InvalidRequest
          */
      void addClockDrift(const SatID& sat, const CommonTime& ttag,
                         const double& drift, const double& sig=0.0)
      {
         try { clkStore.addClockDrift(sat,ttag,drift,sig); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }

         /** Add clock acceleration data (only) to the store
          * @throw InvalidRequest
          */
      void addClockAcceleration(const SatID& sat, const CommonTime& ttag,
                                const double& accel, const double& sig=0.0)
      {
         try { clkStore.addClockAcceleration(sat,ttag,accel,sig); }
         catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
      }


         /// Get number of files (all types) in FileStore.
      int nfiles(void) throw()
      { return (SP3Files.size() + (useSP3clock ? 0 : clkFiles.size())); }

         /// Get number of SP3 files in FileStore.
      int nSP3files(void) throw()
      { return SP3Files.size(); }

         /// Get number of clock files in FileStore.
      int nClockfiles(void) throw()
      { return (useSP3clock ? SP3Files.size() : clkFiles.size()); }


         /// Return true if there is drift data in the tables
      virtual bool hasClockDrift() const throw()
      {  return clkStore.hasClockDrift(); }


         /** Set the flag; if true then all values are rejected when a
          * bad position value is found, while adding data to the
          * store. */
      void rejectBadPositions(const bool flag)
      { rejectBadPosFlag = flag; }

         /** Set the flag; if true then all values are rejected when bad clock
          * values are found, while adding data to the store. */
      void rejectBadClocks(const bool flag)
      { rejectBadClockFlag = flag; }

         /** Set the flag; if true then predicted position values are
          * rejected when adding data to the store. */
      void rejectPredPositions(const bool flag)
      { rejectPredPosFlag = flag; }

         /** Set the flag; if true then predicted clock values are
          * rejected when adding data to the store. */
      void rejectPredClocks(const bool flag)
      { rejectPredClockFlag = flag; }


         /// Is gap checking for position on?
      bool isPosDataGapCheck(void) throw()
      { return posStore.isDataGapCheck(); }

         /// Is gap checking for clock on?
      bool isClkDataGapCheck(void) throw()
      { return clkStore.isDataGapCheck(); }

         /// Disable checking of data gaps in both position and clock.
      void disableDataGapCheck(void) throw()
      { posStore.disableDataGapCheck(); clkStore.disableDataGapCheck(); }

         /// Disable checking of data gaps in position store
      void disablePosDataGapCheck(void) throw()
      { posStore.disableDataGapCheck(); }

         /// Disable checking of data gaps in clock store
      void disableClockDataGapCheck(void) throw()
      { clkStore.disableDataGapCheck(); }

         /// Get current gap interval in the position store.
      double getPosGapInterval(void) throw()
      { return posStore.getGapInterval(); }

         /// Get current gap interval in the clock store.
      double getClockGapInterval(void) throw()
      { return clkStore.getGapInterval(); }

         /** Set gap interval and turn on gap checking in the position
          * store. There is no default. */
      void setPosGapInterval(double interval) throw()
      { posStore.setGapInterval(interval); }

         /** Set gap interval and turn on gap checking in the clock
          * store.  There is no default. */
      void setClockGapInterval(double interval) throw()
      { clkStore.setGapInterval(interval); }


         /// Is interval checking for position on?
      bool isPosIntervalCheck(void) throw()
      { return posStore.isIntervalCheck(); }

         /// Is interval checking for clock on?
      bool isClkIntervalCheck(void) throw()
      { return clkStore.isIntervalCheck(); }

         /// Disable checking of maximum interval in both position and clock.
      void disableIntervalCheck(void) throw()
      { posStore.disableIntervalCheck(); clkStore.disableIntervalCheck(); }

         /// Disable checking of maximum interval in position store
      void disablePosIntervalCheck(void) throw()
      { posStore.disableIntervalCheck(); }

         /// Disable checking of maximum interval in clock store
      void disableClockIntervalCheck(void) throw()
      { clkStore.disableIntervalCheck(); }

         /// Get current maximum interval in the position store
      double getPosMaxInterval(void) throw()
      { return posStore.getMaxInterval(); }

         /// Get current maximum interval in the clock store
      double getClockMaxInterval(void) throw()
      { return clkStore.getMaxInterval(); }

         /** Set maximum interval and turn on interval checking in the
          * position store There is no default. */
      void setPosMaxInterval(double interval) throw()
      { posStore.setMaxInterval(interval); }

         /** Set maximum interval and turn on interval checking in the
          * clock store There is no default. */
      void setClockMaxInterval(double interval) throw()
      { clkStore.setMaxInterval(interval); }


         /// @deprecated
      virtual bool velocityIsPresent() const throw()
      { return posStore.hasVelocity(); }

         /// @deprecated
      virtual bool clockIsPresent() const throw()
      { return true; }

         /** @deprecated
          * Get current (position) interpolation order */
      unsigned int getInterpolationOrder(void) throw()
      { return getPositionInterpOrder(); }

         /** @deprecated
          * Set the (position) interpolation order for the position table;
          * it is forced to be even. */
      void setInterpolationOrder(unsigned int order) throw()
      { setPositionInterpOrder(order); }

   }; // end class SP3EphemerisStore

      //@}

}  // End of namespace gpstk

#endif // GPSTK_SP3_EPHEMERIS_STORE_INCLUDE
