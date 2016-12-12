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

/** \file gameengine/GameLogic/SCA_AddObjectActuator.cpp
 *  \ingroup gamelogic
 *
 * Add an object when this actuator is triggered
 */

#include "SCA_AddObjectActuator.h"
#include "SCA_IScene.h"
#include "KX_GameObject.h"
#include "EXP_PyObjectPlus.h" 


SCA_AddObjectActuator::SCA_AddObjectActuator(SCA_IObject *gameobj,
                                             SCA_IObject *original,
                                             float time,
                                             SCA_IScene *scene,
                                             const float *linvel,
                                             bool linv_local,
                                             const float *angvel,
                                             bool angv_local)
	: SCA_IActuator(gameobj, SCA_ACT_ADD_OBJECT),
      m_OriginalObject(original),
      m_scene(scene),
      m_localLinvFlag(linv_local),
      m_localAngvFlag(angv_local)
{
	m_linear_velocity[0] = linvel[0];
	m_linear_velocity[1] = linvel[1];
	m_linear_velocity[2] = linvel[2];
	m_angular_velocity[0] = angvel[0];
	m_angular_velocity[1] = angvel[1];
	m_angular_velocity[2] = angvel[2];

	if (m_OriginalObject)
		m_OriginalObject->RegisterActuator(this);

	m_lastCreatedObject = NULL;
	m_timeProp = time;
} 

SCA_AddObjectActuator::~SCA_AddObjectActuator()
{ 
	if (m_OriginalObject)
		m_OriginalObject->UnregisterActuator(this);
	if (m_lastCreatedObject)
		m_lastCreatedObject->UnregisterActuator(this);
} 

bool SCA_AddObjectActuator::Update()
{
	bool bNegativeEvent = IsNegativeEvent();
	RemoveAllEvents();

	if (bNegativeEvent)
		return false; // do nothing on negative events

	InstantAddObject();

	return false;
}

SCA_IObject *SCA_AddObjectActuator::GetLastCreatedObject() const
{
	return m_lastCreatedObject;
}

CValue *SCA_AddObjectActuator::GetReplica()
{
	SCA_AddObjectActuator *replica = new SCA_AddObjectActuator(*this);

	if (replica == NULL)
		return NULL;

	// this will copy properties and so on...
	replica->ProcessReplica();

	return replica;
}

void SCA_AddObjectActuator::ProcessReplica()
{
	if (m_OriginalObject)
		m_OriginalObject->RegisterActuator(this);
	m_lastCreatedObject = NULL;
	SCA_IActuator::ProcessReplica();
}

bool SCA_AddObjectActuator::UnlinkObject(SCA_IObject *clientobj)
{
	if (clientobj == m_OriginalObject) {
		// this object is being deleted, we cannot continue to track it.
		m_OriginalObject = NULL;
		return true;
	}
	if (clientobj == m_lastCreatedObject) {
		// this object is being deleted, we cannot continue to track it.
		m_lastCreatedObject = NULL;
		return true;
	}
	return false;
}

void SCA_AddObjectActuator::Relink(std::map<void *, void *> &obj_map)
{
	void *h_obj = obj_map[m_OriginalObject];
	if (h_obj) {
		if (m_OriginalObject)
			m_OriginalObject->UnregisterActuator(this);
		m_OriginalObject = (SCA_IObject *)h_obj;
		m_OriginalObject->RegisterActuator(this);
	}
}

void SCA_AddObjectActuator::InstantAddObject()
{
	if (m_OriginalObject) {
		// Add an identical object, with properties inherited from the original object
		// Now it needs to be added to the current scene.
		SCA_IObject *replica = m_scene->AddReplicaObject(m_OriginalObject, GetParent(), m_timeProp);
		KX_GameObject *game_obj = static_cast<KX_GameObject *>(replica);
		game_obj->setLinearVelocity(MT_Vector3(m_linear_velocity), m_localLinvFlag);
		game_obj->setAngularVelocity(MT_Vector3(m_angular_velocity), m_localAngvFlag);
		game_obj->ResolveCombinedVelocities(MT_Vector3(m_linear_velocity),
		                                    MT_Vector3(m_angular_velocity),
		                                    m_localLinvFlag,
		                                    m_localAngvFlag);

		// keep a copy of the last object, to allow python scripters to change it
		if (m_lastCreatedObject) {
			//Let's not keep a reference to the object: it's bad, if the object is deleted
			//this will force to keep a "zombie" in the game for no good reason.
			//m_scene->DelayedReleaseObject(m_lastCreatedObject);
			//m_lastCreatedObject->Release();

			//Instead we use the registration mechanism
			m_lastCreatedObject->UnregisterActuator(this);
			m_lastCreatedObject = NULL;
		}
		
		m_lastCreatedObject = replica;
		// no reference
		//m_lastCreatedObject->AddRef();
		// but registration
		m_lastCreatedObject->RegisterActuator(this);
		// finished using replica? then release it
		replica->Release();
	}
}

#ifdef WITH_PYTHON

/* ------------------------------------------------------------------------- */
/* Python functions                                                          */
/* ------------------------------------------------------------------------- */

/* Integration hooks ------------------------------------------------------- */
PyTypeObject SCA_AddObjectActuator::Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"SCA_AddObjectActuator",
	sizeof(PyObjectPlus_Proxy),
	0,
	py_base_dealloc,
	0,
	0,
	0,
	0,
	py_base_repr,
	0,0,0,0,0,0,0,0,0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	0,0,0,0,0,0,0,
	Methods,
	0,
	0,
	&SCA_IActuator::Type,
	0,0,0,0,0,0,
	py_base_new
};

PyMethodDef SCA_AddObjectActuator::Methods[] = {
	{"instantAddObject", (PyCFunction) SCA_AddObjectActuator::sPyInstantAddObject, METH_NOARGS, NULL},
	{NULL,NULL} //Sentinel
};

PyAttributeDef SCA_AddObjectActuator::Attributes[] = {
	KX_PYATTRIBUTE_RW_FUNCTION("object", SCA_AddObjectActuator, pyattr_get_object, pyattr_set_object),
	KX_PYATTRIBUTE_RO_FUNCTION("objectLastCreated", SCA_AddObjectActuator, pyattr_get_objectLastCreated),
	KX_PYATTRIBUTE_FLOAT_RW("time", 0.0f, FLT_MAX, SCA_AddObjectActuator, m_timeProp),
	KX_PYATTRIBUTE_FLOAT_ARRAY_RW("linearVelocity", -FLT_MAX,FLT_MAX, SCA_AddObjectActuator, m_linear_velocity, 3),
	KX_PYATTRIBUTE_FLOAT_ARRAY_RW("angularVelocity", -FLT_MAX,FLT_MAX, SCA_AddObjectActuator, m_angular_velocity, 3),
	KX_PYATTRIBUTE_NULL	//Sentinel
};

PyObject *SCA_AddObjectActuator::pyattr_get_object(void *self, const struct KX_PYATTRIBUTE_DEF *attrdef)
{
	SCA_AddObjectActuator *actuator = static_cast<SCA_AddObjectActuator *>(self);
	if (!actuator->m_OriginalObject)
		Py_RETURN_NONE;
	else
		return actuator->m_OriginalObject->GetProxy();
}

int SCA_AddObjectActuator::pyattr_set_object(void *self, const struct KX_PYATTRIBUTE_DEF *attrdef, PyObject *value)
{
	SCA_AddObjectActuator *actuator = static_cast<SCA_AddObjectActuator *>(self);
	KX_GameObject *gameobj;
		
	if (!ConvertPythonToGameObject(actuator->GetLogicManager(), value, &gameobj, true, "actuator.object = value: SCA_AddObjectActuator"))
		return PY_SET_ATTR_FAIL; // ConvertPythonToGameObject sets the error
		
	if (actuator->m_OriginalObject != NULL)
		actuator->m_OriginalObject->UnregisterActuator(actuator);

	actuator->m_OriginalObject = (SCA_IObject *)gameobj;
		
	if (actuator->m_OriginalObject)
		actuator->m_OriginalObject->RegisterActuator(actuator);
		
	return PY_SET_ATTR_SUCCESS;
}

PyObject *SCA_AddObjectActuator::pyattr_get_objectLastCreated(void *self, const struct KX_PYATTRIBUTE_DEF *attrdef)
{
	SCA_AddObjectActuator *actuator = static_cast<SCA_AddObjectActuator *>(self);
	if (!actuator->m_lastCreatedObject)
		Py_RETURN_NONE;
	else
		return actuator->m_lastCreatedObject->GetProxy();
}

PyObject *SCA_AddObjectActuator::PyInstantAddObject()
{
	InstantAddObject();
	Py_RETURN_NONE;
}

#endif // WITH_PYTHON