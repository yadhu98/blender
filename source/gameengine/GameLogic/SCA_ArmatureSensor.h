/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file SCA_ArmatureSensor.h
 *  \ingroup gamelogic
 *  \brief Property sensor
 */

#ifndef __KX_ARMATURESENSOR_H__
#define __KX_ARMATURESENSOR_H__

struct bConstraint;

#include "SCA_ISensor.h"
#include "DNA_sensor_types.h"

class SCA_ArmatureSensor : public SCA_ISensor
{
	Py_Header

private:
	struct bConstraint *m_constraint;
	std::string m_posechannel;
	std::string m_constraintname;
	int m_type;
	float m_value;
	bool m_result;
	bool m_lastresult;

public:
	SCA_ArmatureSensor(class SCA_EventManager *eventmgr,
	                   SCA_IObject *gameobj,
	                   const std::string& posechannel,
	                   const std::string& constraintname,
	                   int type,
	                   float value);

	virtual ~SCA_ArmatureSensor();

	//  For property sensor, it is used to release the pre-calculated expression
	//  so that self references are removed before the sensor itself is released

	virtual CValue *GetReplica();
	virtual void ReParent(SCA_IObject *parent);
	virtual void Init();
	virtual bool Evaluate();
	virtual bool IsPositiveTrigger();

	// identify the constraint that this actuator controls
	void FindConstraint();

#ifdef WITH_PYTHON

	/* --------------------------------------------------------------------- */
	/* Python interface ---------------------------------------------------- */
	/* --------------------------------------------------------------------- */
	static PyObject *pyattr_get_constraint(void *self, const struct KX_PYATTRIBUTE_DEF *attrdef);

#endif  // WITH_PYTHON

};

#endif //__KX_ARMATURESENSOR_H__
