/*
 * Copyright © 2012, United States Government, as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All rights reserved.
 * 
 * The NASA Tensegrity Robotics Toolkit (NTRT) v1 platform is licensed
 * under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
*/

/**
 * @file tgDataManager.cpp
 * @brief Contains the implementations of parts of the abstract class tgDataManager
 * @author Drew Sabelhaus
 * $Id$
 */

// This module
#include "tgDataManager.h"
// This application
#include "tgSensor.h"
#include "core/tgSenseable.h"
#include "tgSensorInfo.h"
// The C++ Standard Library
//#include <stdio.h> // for sprintf
#include <iostream>
#include <stdexcept>
#include <cassert>

/**
 * Nothing to do, in this abstract base class.
 */
tgDataManager::tgDataManager()
{
  //DEBUGGING
  //std::cout << "tgDataManager constructor." << std::endl;
  // Postcondition
  assert(invariant());
}

/**
 * In the destructor, we must be sure to delete everything in both the 
 * m_sensors and m_sensorInfos lists.
 * Ideally, this should be handled by teardown, so this is really a
 * double-check for memory leaks more than anything else.
 */
tgDataManager::~tgDataManager()
{
  //DEBUGGING
  //std::cout << "tgDataManager destructor." << std::endl;
  
  // First, delete everything in m_sensors.
  const size_t n_Sens = m_sensors.size();
  for (size_t i = 0; i < n_Sens; ++i)
  {
    // Pick out one sensor from the list:
    tgSensor* const pSensor = m_sensors[i];
    // TO-DO: check if we need this assert...
    // It is safe to delete NULL, but this is an invariant
    // assert(pChild != NULL); 
    delete pSensor;
    // Null out the deleted pointer.
    m_sensors[i] = NULL;
  }
  // Next, delete the sensor infos.
  const size_t n_SensInf = m_sensorInfos.size();
  for (size_t i = 0; i < n_SensInf; ++i)
  {
    // Pick out one sensorInfo from the list:
    tgSensorInfo* const pSensorInfo = m_sensorInfos[i];
    // TO-DO: check if we need this assert...
    // It is safe to delete NULL, but this is an invariant
    // assert(pChild != NULL); 
    delete pSensorInfo;
    //Null out the deleted pointer.
    m_sensorInfos[i] = NULL;
  }

  // Note that the creation and deletion of the senseable objects, e.g.
  // the tgModels, is handled externally.
  // tgDataManagers should NOT destroy the objects they are sensing.
}

/**
 * Helper for setup.
 * This function abstracts away the loop over the sensor infos list.
 */
void tgDataManager::addSensorsIfAppropriate(tgSenseable* pSenseable)
{
  // Loop over all tgSensorInfos in the list.
  for (size_t i=0; i < m_sensorInfos.size(); i++){
    // If this particular sensor info is appropriate for the pSenseable,
    if( m_sensorInfos[i]->isThisMySenseable(pSenseable) ) {
      // Create a sensor and push it back to our list of sensors.
      m_sensors.push_back( m_sensorInfos[i]->createSensor(pSenseable));
    }
  }
}

/**
 * Create the sensors!
 * Note that it is up to the child classes to do any other type of setup besides
 * sensor creation. For example, a data logger will have to first call this parent
 * function, then ask for the sensor heading from each sensor, then... etc.
 */
void tgDataManager::setup()
{
  // Iterate over all sensables and their descendants, adding sensors
  // if appropriate.
  for (size_t j=0; j < m_senseables.size(); j++){
    // For each senseable object, create sensors for it and its descendants.
    // First, the senseable itself:
    addSensorsIfAppropriate(m_senseables[j]);
    // Then, for all its descendants:
    std::vector<tgSenseable*> descendants =
      m_senseables[j]->getSenseableDescendants();
    // Loop through.
    for (size_t k=0; k < descendants.size(); k++) {
      addSensorsIfAppropriate(descendants[k]);
    }
  }
  
  // Postcondition
  assert(invariant());
}

/**
 * The teardown method has to remove all the sensors and sensor infos.
 * Note that this method could be redefined by subclasses, for example to 
 * do some final step with a mesage-passing algorithm before the simulation ends.
 * TO-DO: what's a good way to capture the deletion of sensors and sensorInfos
 * that doesn't require subclasses to copy-and-paste from this teardown method?
 */
void tgDataManager::teardown()
{
  //DEBUGGING
  //std::cout << "tgDataManager teardown." << std::endl;
  
  // First, delete the sensors.
  // Note that it's good practice to set deleted pointers to NULL here.
  for (std::size_t i = 0; i < m_sensors.size(); i++)
  {
    // Note that sensors don't have any teardown method.
    // The delete method will call their destructors.
    delete m_sensors[i];
    m_sensors[i] = NULL;
  }
  // Clear the list so that the destructor for this class doesn't have to
  // do anything.
  m_sensors.clear();

  // Next, delete the sensor infos.
  // TO-DO: implement this.
  // TO-DO: should we actually be deleting these? Or would that make it so no
  // sensors would be created after a reset???

  // Clear the list of senseable objects.
  // Since tgDataManagers don't ever change these objects,
  // leave it to other classes to create and delete them.
  // Just get rid of the pointers here.
  // TO-DO: maybe we don't want to do this? Will this make it so no logs
  // are created upon reset???
  // TO-DO: could this segfault? Will the pointers in m_senseables be changed
  // during reset at all?
  //m_senseables.clear();

  // Postcondition
  assert(invariant());
  assert(m_sensors.empty());
}

/**
 * The step method is where data is actually collected!
 * This base class won't do anything, but subclasses will re-implement this method.
 */
void tgDataManager::step(double dt) 
{
  //DEBUGGING
  //std::cout << "tgDataManager step." << std::endl;
  if (dt <= 0.0)
  {
    throw std::invalid_argument("dt is not positive");
  }
  else
  {
    // Nothing to do.
  }

  // Postcondition
  assert(invariant());
}

/**
 * This method adds sensor info objects to this data manager.
 * It takes in a pointer to a sensor info and pushes it to the
 * current list of sensor infos.
 */
void tgDataManager::addSensorInfo(tgSensorInfo* pSensorInfo)
{
  // TO-DO:
  // Check if a type of pSensorInfo is already in the list.
  // Otherwise, duplicate sensors will be created: e.g., if two tgRodSensorInfos
  // are in m_sensorInfos, then multiple tgRodSensors will be created for
  // each tgRod.
  //DEBUGGING
  //std::cout << "tgDataManager addSensorInfo." << std::endl;
  // Precondition
  if (pSensorInfo == NULL)
  {
    throw std::invalid_argument("pSensorInfo is NULL inside tgDataManager::addSensorInfo");
  } 

  m_sensorInfos.push_back(pSensorInfo);

  // Postcondition
  assert(invariant());
  assert(!m_sensorInfos.empty());
}

/**
 * This method adds sense-able objects to this data manager.
 * It takes in a pointer to a sense-able object and pushes it to the
 * current list of tgSenseables.
 */
void tgDataManager::addSenseable(tgSenseable* pSenseable)
{
  //DEBUGGING
  //std::cout << "tgDataManager addSenseable." << std::endl;
  // Precondition
  if (pSenseable == NULL)
  {
    throw std::invalid_argument("pSenseable is NULL inside tgDataManager::addSenseable");
  } 

  m_senseables.push_back(pSenseable);

  // Postcondition
  assert(invariant());
  assert(!m_senseables.empty());
}


/**
 * The toString method for data managers should include a list of the number
 * of sensors and sensor Infos it has.
 */
std::string tgDataManager::toString() const
{
  std::string p = "  ";  
  std::ostringstream os;
  // Note that we're using sprintf here to convert an int to a string.
  // TO-DO: fix this!
  os << "tgDataManager"
     << " with " << m_sensors.size() << " sensors, " << m_sensorInfos.size()
     << " sensorInfos, and " << m_senseables.size() << " senseable objects."
     << std::endl;

  /*
    os << prefix << p << "Children:" << std::endl;
  for(std::size_t i = 0; i < m_children.size(); i++) {
    os << m_children[i]->toString(prefix + p) << std::endl;
  }
  os << prefix << p << "Tags: [" << getTags() << "]" << std::endl;
  os << prefix << ")";
  */
  return os.str();
}


bool tgDataManager::invariant() const
{
  // TO-DO:
  // m_sensors and m_sensorInfos are sane, check somehow...?
  return true;
}

std::ostream&
operator<<(std::ostream& os, const tgDataManager& obj)
{
    os << obj.toString() << std::endl;
    return os;
}
