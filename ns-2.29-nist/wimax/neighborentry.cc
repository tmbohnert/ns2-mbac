/* This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * <BR>
 * We would appreciate acknowledgement if the software is used.
 * <BR>
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * </PRE></P>
 * @author  rouil
 */

#include "neighborentry.h"

/** 
 * Constructor
 */
NeighborEntry::NeighborEntry (int id) : nbr_adv_(0), dcd_(0), ucd_ (0), rng_rsp_(0)
{
  id_ = id;
  memset (&state_, 0, sizeof (struct sched_state_info));
  detected_ = false;
}

/**
 * Destructor
 */
NeighborEntry::~NeighborEntry ()
{
  free (nbr_adv_);
  free (dcd_);
  free (ucd_);
  free (rng_rsp_);
}


/**
 * Return the address of the neighbor of this entry
 * @return the address of the neighbor of this entry
 */
int NeighborEntry::getID ()
{
  return id_;
}

/**
 * Set the neighbor advertisement message
 * @param frame The advertisment message
 */
void NeighborEntry::setNbrAdvMessage (mac802_16_nbr_adv_info *frame)
{
  nbr_adv_ = frame;
}
  
/**
 * Return the neighbor advertisement message
 * @param frame The advertisment message
 */
mac802_16_nbr_adv_info * NeighborEntry::getNbrAdvMessage ()
{
  return nbr_adv_;
}

/**
 * Set the DCD message received during scanning
 * @param dcd the DCD message received
 */
void NeighborEntry::setDCD (mac802_16_dcd_frame *frame)
{
  dcd_ = frame;
}

/**
 * Get the DCD message received during scanning
 * @return the DCD message received
 */
mac802_16_dcd_frame * NeighborEntry::getDCD ()
{
  return dcd_;
}

/**
 * Set the UCD message received during scanning
 * @param dcd the DCD message received
 */
void NeighborEntry::setUCD (mac802_16_ucd_frame *frame)
{
  ucd_ = frame;
}

/**
 * Get the DCD message received during scanning
 * @return the DCD message received
 */
mac802_16_ucd_frame * NeighborEntry::getUCD ()
{
  return ucd_;
}

/**
 * Set the MAC state associated with this neighbor
 * @param state
 */
/*void NeighborEntry::setState (sched_state_info *state)
{
  state_ = state;
}*/

/**
 * Get the MAC state associated with this neighbor
 * @return the MAC state associated with this neighbor
 */
sched_state_info * NeighborEntry::getState ()
{
  return &state_;
}

/**
 * Mark the neighbor as being detected
 * @param detected indicate if the neighbor has been detected
 */
void NeighborEntry::setDetected (bool detected)
{
  detected_ = detected;
}

/**
 * Indicates the neighbor as being detected
 * @return indication if the neighbor has been detected
 */
bool NeighborEntry::isDetected ()
{
  return detected_;
}

/**
 * Set the UCD message received during scanning
 * @param dcd the DCD message received
 */
void NeighborEntry::setRangingRsp (mac802_16_rng_rsp_frame *frame)
{
  rng_rsp_ = frame;
}

/**
 * Get the DCD message received during scanning
 * @return the DCD message received
 */
mac802_16_rng_rsp_frame *NeighborEntry::getRangingRsp ()
{
  return rng_rsp_;
}

