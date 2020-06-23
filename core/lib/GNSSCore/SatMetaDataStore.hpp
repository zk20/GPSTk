#ifndef GPSTK_SATMETADATASTORE_HPP
#define GPSTK_SATMETADATASTORE_HPP

#include <map>
#include <set>
#include "SatMetaData.hpp"
#include "SatMetaDataSort.hpp"
#include "NavID.hpp"

namespace gpstk
{
      /** Provide a class for reading satellite metadata from a CSV
       * file and provide methods for looking up information in that
       * file.
       */
   class SatMetaDataStore
   {
   public:
         /// Specifies a single GNSS signal.
      struct Signal
      {
         ObsID::CarrierBand carrier; ///< Carrier frequency.
         ObsID::TrackingCode code;   ///< Tracking code.
         NavID::NavType nav;         ///< Navigation code.
      };
         /// Set of signals that may be transmitted by a satellite.
      using SignalSet = std::set<Signal>;
         /// Map of signal set name to signal set.
      using SignalMap = std::map<std::string, SignalSet>;
         /// Set of satellites ordered by PRN or channel/slotID.
      using SatSet = std::multiset<SatMetaData, SatMetaDataSort>;
         /// Satellites grouped by system.
      using SatMetaMap = std::map<SatelliteSystem, SatSet>;

         /// Nothin doin.
      SatMetaDataStore() = default;

         /** Attempt to load satellite metadata from the store.
          * The format of the input file is CSV, the values being
          *   \li SAT (literal)
          *   \li prn
          *   \li svn
          *   \li NORAD ID
          *   \li FDMA channel
          *   \li slot ID
          *   \li GNSS name
          *   \li launch time year
          *   \li launch time day of year
          *   \li launch time seconds of day
          *   \li start time year
          *   \li start time day of year
          *   \li start time seconds of day
          *   \li end time year
          *   \li end time day of year
          *   \li end time seconds of day
          *   \li plane
          *   \li slot
          *   \li type/block
          *   \li signal set name
          *   \li mission number
          *   \li satellite status
          *   \li clock type 1
          *   \li clock type 2
          *   \li clock type 3
          *   \li clock type 4
          *   \li active clock number
          *
          * Signal sets are defined using multiple SIG records as follows
          *   \li SIG (literal)
          *   \li signal set name
          *   \li carrier band name
          *   \li tracking code name
          *   \li navigation code name
          *
          * @param[in] sourceName The path to the input CSV-format file.
          * @return true if successful, false on error.
          */
      virtual bool loadData(const std::string& sourceName);

         /** Find a satellite in the map by searching by PRN.
          * @param[in] sys The GNSS of the desired satellite.
          * @param[in] prn The pseudo-random number identifying the
          *   desired satellite.
          * @param[in] when The time of interest of the desired satellite.
          * @param[out] sat If found the satellite's metadata.
          * @return true if the requested satellite mapping was found.
          */
      bool findSat(SatelliteSystem sys, uint32_t prn,
                   const gpstk::CommonTime& when,
                   SatMetaData& sat)
         const;

         /** Find a satellite in the map by searching by PRN.
          * @param[in] prn The satellite to find, identified by PRN
          *   (i.e. not FDMA channel/slot).
          * @param[in] when The time of interest of the desired satellite.
          * @param[out] sat If found the satellite's metadata.
          * @return true if the requested satellite mapping was found.
          */
      bool findSat(const SatID& prn,
                   const gpstk::CommonTime& when,
                   SatMetaData& sat)
         const
      { return findSat(prn.system, prn.id, when, sat); }

         /** Get the space vehicle number of a satellite in the map by
          * searching by PRN.
          * @param[in] sys The GNSS of the desired satellite.
          * @param[in] prn The pseudo-random number identifying the
          *   desired satellite.
          * @param[in] when The time of interest of the desired satellite.
          * @param[out] svn If found the satellite's vehicle number.
          * @return true if the requested satellite mapping was found.
          */
      bool getSVN(SatelliteSystem sys, uint32_t prn,
                  const gpstk::CommonTime& when,
                  std::string& svn)
         const;

         /** Get the space vehicle number of a satellite in the map by
          * searching by PRN.
          * @param[in] sat The ID of the desired satellite.
          * @param[in] when The time of interest of the desired satellite.
          * @param[out] svn If found the satellite's vehicle number.
          * @return true if the requested satellite mapping was found.
          */
      bool getSVN(const SatID& sat, const gpstk::CommonTime& when,
                  std::string& svn)
         const
      { return getSVN(sat.system, sat.id, when, svn); }

         /** Find a satellite in the map by searching by SVN.
          * @param[in] sys The GNSS of the desired satellite.
          * @param[in] svn The system-unique space vehicle number
          *   identifying the desired satellite.
          * @param[in] when The time of interest of the desired satellite.
          * @param[out] sat If found the satellite's metadata.
          * @return true if the requested satellite mapping was found.
          */
      bool findSatBySVN(SatelliteSystem sys, const std::string& svn,
                        const gpstk::CommonTime& when,
                        SatMetaData& sat)
         const;

         /** Get the pseudo-random number of a satellite in the map by
          * searching by SVN.
          * @param[in] sys The GNSS of the desired satellite.
          * @param[in] svn The space vehicle number identifying the
          *   desired satellite.
          * @param[in] when The time of interest of the desired satellite.
          * @param[out] prn If found the satellite's pseudo-random number.
          * @return true if the requested satellite mapping was found.
          */
      bool getPRN(SatelliteSystem sys, const std::string& svn,
                  const gpstk::CommonTime& when,
                  uint32_t& prn)
         const;

         /// Storage of all the satellite metadata.
      SatMetaMap satMap;
         /// Map signal set name to the actual signals.
      SignalMap sigMap;

   protected:
         /** Convert a SAT record to a SatMetaData record and store it.
          * @param[in] vals SAT record in the form of an array of columns.
          * @return true if successful, false on error.
          */
      bool addSat(const std::vector<std::string>& vals);
         /** Convert a SIG record to a Signal object and store it.
          * @param[in] vals SIG record in the form of an array of columns.
          * @return true if successful, false on error.
          */
      bool addSignal(const std::vector<std::string>& vals);
   }; // class SatMetaDataStore

} // namespace gpstk

#endif // GPSTK_SATMETADATASTORE_HPP
