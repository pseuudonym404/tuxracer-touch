/* --------------------------------------------------------------------
EXTREME TUXRACER

Copyright (C) 1999-2001 Jasmin F. Patry (Tuxracer)
Copyright (C) 2010 Extreme Tuxracer Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


This module has been completely rewritten. Remember that the way of
defining the character has radically changed though the character is
still shaped with spheres.
---------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include <etr_config.h>
#endif

#include "tux.h"
#include "ogl.h"
#include "spx.h"
#include "textures.h"
#include "course.h"
#include "physics.h"
//#include <GL/glu.h>
#include <algorithm>

#define MAX_ARM_ANGLE2 30.0
#define MAX_PADDLING_ANGLE2 35.0
#define MAX_EXT_PADDLING_ANGLE2 30.0
#define MAX_KICK_PADDLING_ANGLE2 20.0
#define TO_AIR_TIME 0.5
#define TO_TIME 0.14

#define SHADOW_HEIGHT 0.03 // ->0.05

#ifdef USE_STENCIL_BUFFER
static const TColor shad_col(0.0, 0.0, 0.0, 0.3);
#else
static const TColor shad_col(0.0, 0.0, 0.0, 0.1);
#endif

static const TCharMaterial TuxDefMat = {TColor(0.5, 0.5, 0.5, 1.0), TColor(0.0, 0.0, 0.0, 1.0), 0.0, string()};
static const TCharMaterial Highlight = {TColor(0.8, 0.15, 0.15, 1.0), TColor(0.0, 0.0, 0.0, 1.0), 0.0, string()};

static int numpoints = 0;
static bool bVerticesLoaded = false;
static GLfloat charvertices[30000];
static GLushort charvertexcount[10000];

CCharShape TestChar;

CCharShape::CCharShape () {
	for (int i=0; i<MAX_CHAR_NODES; i++) {
		Nodes[i] = NULL;
		Index[i] = -1;
	}
	numNodes = 0;

	useActions = false;
	newActions = false;
	useMaterials = true;
	useHighlighting = false;
	highlighted = false;
	highlight_node = -1;

	if (!bVerticesLoaded)
	{
		float fTheta = 32 * M_PI/180.0;
		float fPhi = fTheta;
		int pos = 0;
		
		for(float phi = -(M_PI); phi < M_PI; phi+=fPhi) 
		{
			for(float theta = 0.0; theta < (2*M_PI); theta+=fTheta) 
			{
				charvertices[pos++] = (sin(phi) * cos(theta));
				charvertices[pos++] = (sin(phi) * sin(theta));
				charvertices[pos++] = (cos(phi));
				charvertexcount[numpoints] = numpoints;
				numpoints++;
				charvertices[pos++] = (sin(phi+fPhi) * cos(theta));
				charvertices[pos++] = (sin(phi+fPhi) * sin(theta));
				charvertices[pos++] = (cos(phi+fPhi));
				charvertexcount[numpoints] = numpoints;
				numpoints++;
			}
		}
		bVerticesLoaded = true;
	}
}

CCharShape::~CCharShape() {
	for (int i=0; i<MAX_CHAR_NODES; i++) {
		if (Nodes[i] != NULL) {
			delete Nodes[i]->action;
			delete Nodes[i];
		}
	}
}

// --------------------------------------------------------------------
//				nodes
// --------------------------------------------------------------------

size_t CCharShape::GetNodeIdx (size_t node_name) const {
	if (node_name >= MAX_CHAR_NODES) return -1;
	return Index[node_name];
}

TCharNode *CCharShape::GetNode (size_t node_name) {
	size_t idx = GetNodeIdx (node_name);
	if (idx >= numNodes) return NULL;
	return Nodes[idx];
}

void CCharShape::CreateRootNode () {
	TCharNode *node = new TCharNode;
	node->node_name = 0;
	node->parent = NULL;
	node->parent_name = 99;
	node->next = NULL;
	node->next_name = 99;
	node->child = NULL;
	node->child_name = 99;
	node->mat = NULL;
	node->joint = "root";
	node->render_shadow = false;
	node->visible = false;
	node->action = NULL;
	node->trans.SetIdentity();
	node->invtrans.SetIdentity();

	NodeIndex.clear();
	NodeIndex["root"] = 0;
	Index[0] = 0;
	Nodes[0] = node;
	numNodes = 1;
}

bool CCharShape::CreateCharNode(int parent_name, size_t node_name, const string& joint, const string& name, const string& order, bool shadow) {
	TCharNode *parent = GetNode (parent_name);
	if (parent == NULL) {
		Message ("wrong parent node");
		return false;
	}
	TCharNode *node = new TCharNode;
	node->node_name = node_name;
	node->parent = parent;
	node->parent_name = parent_name;
	node->next  = NULL;
	node->next_name = 99;
	node->child = NULL;
	node->child_name = 99;

	if (useActions) {
		node->action = new TCharAction;
		node->action->num = 0;
		node->action->name = name;
		node->action->order = order;
		node->action->mat = "";
	} else
		node->action = NULL;

	node->mat   = NULL;
	node->node_idx = numNodes;
	node->visible = false;
	node->render_shadow = shadow;
	node->joint = joint;

	node->trans.SetIdentity();
	node->invtrans.SetIdentity();

	if (!joint.empty()) NodeIndex[joint] = node_name;
	Nodes[numNodes] = node;
	Index[node_name] = numNodes;

/// -------------------------------------------------------------------
	if (parent->child == NULL) {
		parent->child = node;
		parent->child_name = node_name;
	} else {
		for (parent = parent->child; parent->next != NULL; parent = parent->next) {}
		parent->next = node;
		parent->next_name = node_name;
	}
/// -------------------------------------------------------------------

	numNodes++;
	return true;
}

void CCharShape::AddAction (size_t node_name, int type, const TVector3d& vec, ETR_DOUBLE val) {
	size_t idx = GetNodeIdx (node_name);
	TCharAction *act = Nodes[idx]->action;
	act->type[act->num] = type;
	act->vec[act->num] = vec;
	act->dval[act->num] = val;
	act->num++;
}

bool CCharShape::TranslateNode (size_t node_name, const TVector3d& vec) {
	TCharNode *node = GetNode (node_name);
	if (node == NULL) return false;

	TMatrix<4, 4> TransMatrix;

	TransMatrix.SetTranslationMatrix(vec.x, vec.y, vec.z);
	node->trans = node->trans * TransMatrix;
	TransMatrix.SetTranslationMatrix(-vec.x, -vec.y, -vec.z);
	node->invtrans = TransMatrix * node->invtrans;

	if (newActions && useActions) AddAction (node_name, 0, vec, 0);
	return true;
}

bool CCharShape::RotateNode (size_t node_name, int axis, ETR_DOUBLE angle) {
	TCharNode *node = GetNode (node_name);
	if (node == NULL) return false;

	if (axis > 3) return false;

	TMatrix<4, 4> rotMatrix;
	char caxis = '0';
	switch (axis) {
		case 1:
			caxis = 'x';
			break;
		case 2:
			caxis = 'y';
			break;
		case 3:
			caxis = 'z';
			break;
	}

	rotMatrix.SetRotationMatrix(angle, caxis);
	node->trans = node->trans * rotMatrix;
	rotMatrix.SetRotationMatrix(-angle, caxis);
	node->invtrans = rotMatrix * node->invtrans;

	if (newActions && useActions) AddAction (node_name, axis, NullVec3, angle);
	return true;
}

bool CCharShape::RotateNode (const string& node_trivialname, int axis, ETR_DOUBLE angle) {
	map<string, size_t>::const_iterator i = NodeIndex.find(node_trivialname);
	if (i == NodeIndex.end()) return false;
	return RotateNode (i->second, axis, angle);
}

void CCharShape::ScaleNode (size_t node_name, const TVector3d& vec) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return;

	TMatrix<4, 4> matrix;

	matrix.SetScalingMatrix(vec.x, vec.y, vec.z);
	node->trans = node->trans * matrix;
	matrix.SetScalingMatrix(1.0 / vec.x, 1.0 / vec.y, 1.0 / vec.z);
	node->invtrans = matrix * node->invtrans;

	if (newActions && useActions) AddAction (node_name, 4, vec, 0);
}

bool CCharShape::VisibleNode (size_t node_name, float level) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;

	node->visible = (level > 0);

	if (node->visible) {
		node->divisions =
		    clamp (MIN_SPHERE_DIV, ROUND_TO_NEAREST (param.tux_sphere_divisions * level / 10), MAX_SPHERE_DIV);
		node->radius = 1.0;
	}
	if (newActions && useActions) AddAction (node_name, 5, NullVec3, level);
	return true;
}

bool CCharShape::MaterialNode (size_t node_name, const string& mat_name) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;
	TCharMaterial *mat = GetMaterial(mat_name);
	if (mat == NULL) return false;
	node->mat = mat;
	if (newActions && useActions) node->action->mat = mat_name;
	return true;
}

bool CCharShape::ResetNode (size_t node_name) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;

	node->trans.SetIdentity();
	node->invtrans.SetIdentity();
	return true;
}

bool CCharShape::ResetNode (const string& node_trivialname) {
	map<string, size_t>::const_iterator i = NodeIndex.find(node_trivialname);
	if (i == NodeIndex.end()) return false;
	return ResetNode (i->second);
}

bool CCharShape::TransformNode(size_t node_name, const TMatrix<4, 4>& mat, const TMatrix<4, 4>& invmat) {
	TCharNode *node = GetNode(node_name);
	if (node == NULL) return false;

	node->trans = node->trans * mat;
	node->invtrans = invmat * node->invtrans;
	return true;
}

void CCharShape::ResetJoints () {
	ResetNode ("left_shldr");
	ResetNode ("right_shldr");
	ResetNode ("left_hip");
	ResetNode ("right_hip");
	ResetNode ("left_knee");
	ResetNode ("right_knee");
	ResetNode ("left_ankle");
	ResetNode ("right_ankle");
	ResetNode ("tail");
	ResetNode ("neck");
	ResetNode ("head");
}

void CCharShape::Reset () {
	for (int i=0; i<MAX_CHAR_NODES; i++) {
		if (Nodes[i] != NULL) {
			delete Nodes[i]->action;
			delete Nodes[i];
			Nodes[i] = NULL;
		}
		Index[i] = -1;
	}
	Materials.clear();
	NodeIndex.clear();
	MaterialIndex.clear();
	numNodes = 0;

	useActions = true;
	newActions = false;
	useMaterials = true;
	useHighlighting = false;
	highlighted = false;
	highlight_node = -1;
}

// --------------------------------------------------------------------
//				materials
// --------------------------------------------------------------------

TCharMaterial* CCharShape::GetMaterial (const string& mat_name) {
	map<string, size_t>::const_iterator i = MaterialIndex.find(mat_name);
	if (i != MaterialIndex.end() && i->second < Materials.size()) {
		return &Materials[i->second];
	}
	return NULL;
}

void CCharShape::CreateMaterial (const string& line) {
	TVector3d diff = SPVector3d(line, "diff");
	TVector3d spec = SPVector3d(line, "spec");
	float exp = SPFloatN (line, "exp", 50);
	std::string mat = SPStrN(line, "mat");

	Materials.push_back(TCharMaterial());
	Materials.back().diffuse.r = diff.x;
	Materials.back().diffuse.g = diff.y;
	Materials.back().diffuse.b = diff.z;
	Materials.back().diffuse.a = 1.0;
	Materials.back().specular.r = spec.x;
	Materials.back().specular.g = spec.y;
	Materials.back().specular.b = spec.z;
	Materials.back().specular.a = 1.0;
	Materials.back().exp = exp;
	if (useActions)
		Materials.back().matline = line;

	MaterialIndex[mat] = Materials.size()-1;
}

// --------------------------------------------------------------------
//				drawing
// --------------------------------------------------------------------

void CCharShape::DrawCharSphere (int num_divisions) {
	if (num_divisions<1)
		return;
	
	glDrawElements(GL_TRIANGLE_STRIP, numpoints, GL_UNSIGNED_INT, charvertexcount);
}

void CCharShape::DrawNodes (const TCharNode *node) {
	glPushMatrix();
	glMultMatrix(node->trans);

	if (node->node_name == highlight_node) highlighted = true;
	const TCharMaterial *mat;
	if (highlighted && useHighlighting) {
		mat = &Highlight;
	} else {
		if (node->mat != NULL && useMaterials) mat = node->mat;
		else mat = &TuxDefMat;
	}

	if (node->visible == true) {
		set_material (mat->diffuse, mat->specular, mat->exp);

		DrawCharSphere (node->divisions);
	}
// -------------- recursive loop -------------------------------------
	TCharNode *child = node->child;
	while (child != NULL) {
		DrawNodes (child);
		if (child->node_name == highlight_node) highlighted = false;
		child = child->next;
	}
// -------------------------------------------------------------------
	glPopMatrix();
}

void CCharShape::Draw () {
	static const float dummy_color[] = {0.0, 0.0, 0.0, 1.0};

	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dummy_color);
	ScopedRenderMode rm(TUX);
	glEnable (GL_NORMALIZE);

	TCharNode *node = GetNode(0);
	if (node == NULL) return;

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, 0,charvertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0,charvertices);
	DrawNodes (node);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable (GL_NORMALIZE);
	if (param.perf_level > 2 && g_game.argument == 0) DrawShadow ();
	highlighted = false;
}

// --------------------------------------------------------------------

bool CCharShape::Load (const string& dir, const string& filename, bool with_actions) {
	CSPList list (500);

	useActions = with_actions;
	CreateRootNode ();
	newActions = true;

	if (!list.Load (dir, filename)) {
		Message ("could not load character", filename);
		return false;
	}

	for (size_t i=0; i<list.Count(); i++) {
		const string& line = list.Line(i);
		int node_name = SPIntN (line, "node", -1);
		int parent_name = SPIntN (line, "par", -1);
		string mat_name = SPStrN (line, "mat");
		string name = SPStrN (line, "joint");
		string fullname = SPStrN (line, "name");

		if (SPIntN (line, "material", 0) > 0) {
			CreateMaterial (line);
		} else {
			float visible = SPFloatN (line, "vis", -1.0);
			bool shadow = SPBoolN (line, "shad", false);
			string order = SPStrN (line, "order");
			CreateCharNode (parent_name, node_name, name, fullname, order, shadow);
			TVector3d rot = SPVector3d(line, "rot");
			MaterialNode (node_name, mat_name);
			for (size_t ii = 0; ii < order.size(); ii++) {
				int act = order[ii]-48;
				switch (act) {
					case 0: {
						TVector3d trans = SPVector3d(line, "trans");
						TranslateNode (node_name, trans);
						break;
					}
					case 1:
						RotateNode (node_name, 1, rot.x);
						break;
					case 2:
						RotateNode (node_name, 2, rot.y);
						break;
					case 3:
						RotateNode (node_name, 3, rot.z);
						break;
					case 4: {
						TVector3d scale = SPVector3(line, "scale", TVector3d(1, 1, 1));
						ScaleNode (node_name, scale);
						break;
					}
					case 5:
						VisibleNode (node_name, visible);
						break;
					case 9:
						RotateNode (node_name, 2, rot.z);
						break;
					default:
						break;
				}
			}
		}
	}
	newActions = false;
	return true;
}

TVector3d CCharShape::AdjustRollvector (const CControl *ctrl, const TVector3d& vel_, const TVector3d& zvec) {
	TMatrix<4, 4> rot_mat;
	TVector3d vel = ProjectToPlane(zvec, vel_);
	vel.Norm();
	if (ctrl->is_braking) {
		rot_mat = RotateAboutVectorMatrix (vel, ctrl->turn_fact * BRAKING_ROLL_ANGLE);
	} else {
		rot_mat = RotateAboutVectorMatrix (vel, ctrl->turn_fact * MAX_ROLL_ANGLE);
	}
	return TransformVector (rot_mat, zvec);
}

void CCharShape::AdjustOrientation (CControl *ctrl, bool eps,
                                    ETR_DOUBLE dist_from_surface, const TVector3d& surf_nml) {
	TVector3d new_y, new_z;
	static const TVector3d minus_z_vec(0, 0, -1);
	static const TVector3d y_vec(0, 1, 0);

	if (dist_from_surface > 0) {
		new_y = ctrl->cvel;
		new_y.Norm();
		new_z = ProjectToPlane (new_y, TVector3d(0, -1, 0));
		new_z.Norm();
		new_z = AdjustRollvector (ctrl, ctrl->cvel, new_z);
	} else {
		new_z = -1.0 * surf_nml;
		new_z = AdjustRollvector (ctrl, ctrl->cvel, new_z);
		new_y = ProjectToPlane (surf_nml, ctrl->cvel);
		new_y.Norm();
	}

	TVector3d new_x = CrossProduct (new_y, new_z);
	TMatrix<4, 4> cob_mat(new_x, new_y, new_z);
	TQuaternion new_orient = MakeQuaternionFromMatrix (cob_mat);

	if (!ctrl->orientation_initialized) {
		ctrl->orientation_initialized = true;
		ctrl->corientation = new_orient;
	}

	ETR_DOUBLE time_constant = dist_from_surface > 0 ? TO_AIR_TIME : TO_TIME;

	float dtime = eps ? EPS : g_game.time_step;
	ctrl->corientation = InterpolateQuaternions (
	                         ctrl->corientation, new_orient,
	                         min (dtime / time_constant, 1.0));

	ctrl->plane_nml = RotateVector (ctrl->corientation, minus_z_vec);
	ctrl->cdirection = RotateVector (ctrl->corientation, y_vec);
	cob_mat = MakeMatrixFromQuaternion(ctrl->corientation);

	// Trick rotations
	new_y = TVector3d (cob_mat[1][0], cob_mat[1][1], cob_mat[1][2]);
	TMatrix<4, 4> rot_mat = RotateAboutVectorMatrix(new_y, (ctrl->roll_factor * 360));
	cob_mat = rot_mat * cob_mat;
	new_x = TVector3d (cob_mat[0][0], cob_mat[0][1], cob_mat[0][2]);
	rot_mat = RotateAboutVectorMatrix (new_x, ctrl->flip_factor * 360);
	cob_mat = rot_mat * cob_mat;

	TransformNode (0, cob_mat, cob_mat.GetTransposed());
}

void CCharShape::AdjustJoints (ETR_DOUBLE turnFact, bool isBraking,
                               ETR_DOUBLE paddling_factor, ETR_DOUBLE speed,
                               const TVector3d& net_force, ETR_DOUBLE flap_factor) {
	ETR_DOUBLE turning_angle[2];
	ETR_DOUBLE paddling_angle = 0;
	ETR_DOUBLE ext_paddling_angle = 0;
	ETR_DOUBLE kick_paddling_angle = 0;
	ETR_DOUBLE braking_angle = 0;
	ETR_DOUBLE force_angle = 0;
	ETR_DOUBLE turn_leg_angle = 0;
	ETR_DOUBLE flap_angle = 0;

	if (isBraking) braking_angle = MAX_ARM_ANGLE2;

	paddling_angle = MAX_PADDLING_ANGLE2 * sin(paddling_factor * M_PI);
	ext_paddling_angle = MAX_EXT_PADDLING_ANGLE2 * sin(paddling_factor * M_PI);
	kick_paddling_angle = MAX_KICK_PADDLING_ANGLE2 * sin(paddling_factor * M_PI * 2.0);

	turning_angle[0] = max(-turnFact,0.0) * MAX_ARM_ANGLE2;
	turning_angle[1] = max(turnFact,0.0) * MAX_ARM_ANGLE2;
	flap_angle = MAX_ARM_ANGLE2 * (0.5 + 0.5 * sin (M_PI * flap_factor * 6 - M_PI / 2));
	force_angle = clamp (-20.0, -net_force.z / 300.0, 20.0);
	turn_leg_angle = turnFact * 10;

	ResetJoints ();

	RotateNode ("left_shldr", 3,
	            min (braking_angle + paddling_angle + turning_angle[0], MAX_ARM_ANGLE2) + flap_angle);
	RotateNode ("right_shldr", 3,
	            min (braking_angle + paddling_angle + turning_angle[1], MAX_ARM_ANGLE2) + flap_angle);

	RotateNode ("left_shldr", 2, -ext_paddling_angle);
	RotateNode ("right_shldr", 2, ext_paddling_angle);
	RotateNode ("left_hip", 3, -20 + turn_leg_angle + force_angle);
	RotateNode ("right_hip", 3, -20 - turn_leg_angle + force_angle);

	RotateNode ("left_knee", 3,
	            -10 + turn_leg_angle - min (35.0f, speed) + kick_paddling_angle + force_angle);
	RotateNode ("right_knee", 3,
	            -10 - turn_leg_angle - min (35.0f, speed) - kick_paddling_angle + force_angle);

	RotateNode ("left_ankle", 3, -20 + min (50.0f, speed));
	RotateNode ("right_ankle", 3, -20 + min (50.0f, speed));
	RotateNode ("tail", 3, turnFact * 20);
	RotateNode ("neck", 3, -50);
	RotateNode ("head", 3, -30);
	RotateNode ("head", 2, -turnFact * 70);
}

// --------------------------------------------------------------------
//				collision
// --------------------------------------------------------------------

bool CCharShape::CheckPolyhedronCollision(const TCharNode *node, const TMatrix<4, 4>& modelMatrix,
        const TMatrix<4, 4>& invModelMatrix, const TPolyhedron& ph) {
	bool hit = false;

	TMatrix<4, 4> newModelMatrix = modelMatrix * node->trans;
	TMatrix<4, 4> newInvModelMatrix = node->invtrans * invModelMatrix;

	if (node->visible) {
		TPolyhedron newph = ph;
		TransPolyhedron (newInvModelMatrix, newph);
		hit = IntersectPolyhedron (newph);
	}

	if (hit == true) return hit;
	const TCharNode *child = node->child;
	while (child != NULL) {
		hit = CheckPolyhedronCollision (child, newModelMatrix, newInvModelMatrix, ph);
		if (hit == true) return hit;
		child = child->next;
	}
	return false;
}

bool CCharShape::CheckCollision (const TPolyhedron& ph) {
	TCharNode *node = GetNode(0);
	if (node == NULL) return false;
	const TMatrix<4, 4>& identity = TMatrix<4, 4>::getIdentity();
	return CheckPolyhedronCollision(node, identity, identity, ph);
}

bool CCharShape::Collision (const TVector3d& pos, const TPolyhedron& ph) {
	ResetNode (0);
	TranslateNode (0, TVector3d (pos.x, pos.y, pos.z));
	return CheckCollision (ph);
}

// --------------------------------------------------------------------
//				shadow
// --------------------------------------------------------------------
bool bLoaded = false;
ETR_DOUBLE vectors[32*3];

void CCharShape::DrawShadowVertex(int& n, const TMatrix<4, 4>& mat) {
	TVector3d pt(vectors[n],vectors[n+1] , vectors[n+2]);
	n += 3;
	pt = TransformPoint (mat, pt);
	ETR_DOUBLE old_y = pt.y;
	//TVector3d nml = Course.FindCourseNormal (pt.x, pt.z);
	pt.y = Course.FindYCoord (pt.x, pt.z) + SHADOW_HEIGHT;
	if (pt.y > old_y) pt.y = old_y;
	//glNormal3(nml);
	glVertex3(pt);
}

void CCharShape::DrawShadowSphere(const TMatrix<4, 4>& mat) {
	int vc  = 0;
	if (!bLoaded)
	{
		ETR_DOUBLE theta, phi, d_theta, d_phi, eps, twopi;
		//int div = param.tux_shadow_sphere_divisions;
		eps = 1e-15;
		twopi = M_PI * 2.0;
		d_theta = d_phi = M_PI / 3;
		for (phi = 0.0; phi + eps < M_PI; phi += d_phi) {
			ETR_DOUBLE cos_theta, sin_theta;
			ETR_DOUBLE sin_phi, cos_phi;
			ETR_DOUBLE sin_phi_d_phi, cos_phi_d_phi;
			
			sin_phi = sin (phi);
			cos_phi = cos (phi);
			sin_phi_d_phi = sin (phi + d_phi);
			cos_phi_d_phi = cos (phi + d_phi);
			
			if (phi <= eps) {
				vectors[vc++] = 0.0;
				vectors[vc++] = 0.0;
				vectors[vc++] = 1.0;
				for (theta = 0.0; theta + eps < twopi; theta += d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
					
					vectors[vc++] = cos_theta * sin_phi_d_phi;
					vectors[vc++] = sin_theta * sin_phi_d_phi;
					vectors[vc++] = cos_phi_d_phi;
				}
				vectors[vc++] = sin_phi_d_phi;
				vectors[vc++] = 0.0;
				vectors[vc++] = cos_phi_d_phi;
			} else if (phi + d_phi + eps >= M_PI) {
				vectors[vc++] = 0.0;
				vectors[vc++] = 0.0;
				vectors[vc++] = -1.0;
				for (theta = twopi; theta - eps > 0; theta -= d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
					vectors[vc++] = cos_theta * sin_phi;
					vectors[vc++] = sin_theta * sin_phi;
					vectors[vc++] = cos_phi;
				}
				vectors[vc++] = sin_phi;
				vectors[vc++] = 0.0;
				vectors[vc++] = cos_phi;
			} else {
				for (theta = 0.0; theta + eps < twopi; theta += d_theta) {
					sin_theta = sin (theta);
					cos_theta = cos (theta);
					vectors[vc++] = cos_theta * sin_phi;
					vectors[vc++] = sin_theta * sin_phi;
					vectors[vc++] = cos_phi;
					
					vectors[vc++] = cos_theta * sin_phi_d_phi;
					vectors[vc++] = sin_theta * sin_phi_d_phi;
					vectors[vc++] = cos_phi_d_phi;
				}
				vectors[vc++] = sin_phi;
				vectors[vc++] = 0.0;
				vectors[vc++] = cos_phi;
				vectors[vc++] = sin_phi_d_phi;
				vectors[vc++] = 0.0;
				vectors[vc++] = cos_phi_d_phi;
			}
		}
		bLoaded = true;
	}
	vc  = 0;
	glBegin (GL_TRIANGLE_FAN);
	DrawShadowVertex (vc, mat);
	for (int x = 0;x < 7; x++) {
		DrawShadowVertex (vc, mat);
	}
	DrawShadowVertex (vc, mat);
	glEnd();
	glBegin (GL_TRIANGLE_STRIP);
	for (int x = 0;x < 7; x++) {
		DrawShadowVertex (vc, mat);
		DrawShadowVertex (vc, mat);
	}
	DrawShadowVertex (vc, mat);
	DrawShadowVertex (vc, mat);
	glEnd();
	glBegin (GL_TRIANGLE_FAN);
	DrawShadowVertex (vc, mat);
	for (int x = 0;x < 6; x++) {
		DrawShadowVertex (vc, mat);
	}
	DrawShadowVertex (vc, mat);
	glEnd();
}

void CCharShape::TraverseDagForShadow(const TCharNode *node, const TMatrix<4, 4>& mat) {
	TMatrix<4, 4> new_matrix = mat * node->trans;
	if (node->visible && node->render_shadow)
		DrawShadowSphere (new_matrix);

	TCharNode* child = node->child;
	while (child != NULL) {
		TraverseDagForShadow (child, new_matrix);
		child = child->next;
	}
}

void CCharShape::DrawShadow () {
	if (g_game.light_id == 1 || g_game.light_id == 3) return;

	ScopedRenderMode rm(TUX_SHADOW);
	glColor(shad_col);

	TCharNode *node = GetNode(0);
	if (node == NULL) {
		Message ("couldn't find tux's root node");
		return;
	}
	TraverseDagForShadow(node, TMatrix<4, 4>::getIdentity());
}

// --------------------------------------------------------------------
//				testing and tools
// --------------------------------------------------------------------

string CCharShape::GetNodeJoint (size_t idx) const {
	if (idx >= numNodes) return "";
	TCharNode *node = Nodes[idx];
	if (node == NULL) return "";
	if (!node->joint.empty()) return node->joint;
	else return Int_StrN ((int)node->node_name);
}

size_t CCharShape::GetNodeName (size_t idx) const {
	if (idx >= numNodes) return -1;
	return Nodes[idx]->node_name;
}

size_t CCharShape::GetNodeName (const string& node_trivialname) const {
	return NodeIndex.at(node_trivialname);
}


void CCharShape::RefreshNode (size_t idx) {
	if (idx >= numNodes) return;
	TMatrix<4, 4> TempMatrix;
	char caxis;
	ETR_DOUBLE angle;

	TCharNode *node = Nodes[idx];
	TCharAction *act = node->action;
	if (act == NULL) return;
	if (act->num < 1) return;

	node->trans.SetIdentity();
	node->invtrans.SetIdentity();

	for (size_t i=0; i<act->num; i++) {
		int type = act->type[i];
		const TVector3d& vec = act->vec[i];
		ETR_DOUBLE dval = act->dval[i];

		switch (type) {
			case 0:
				TempMatrix.SetTranslationMatrix(vec.x, vec.y, vec.z);
				node->trans = node->trans * TempMatrix;
				TempMatrix.SetTranslationMatrix(-vec.x, -vec.y, -vec.z);
				node->invtrans = TempMatrix * node->invtrans;
				break;
			case 1:
				caxis = 'x';
				angle = dval;
				TempMatrix.SetRotationMatrix(angle, caxis);
				node->trans = node->trans * TempMatrix;
				TempMatrix.SetRotationMatrix(-angle, caxis);
				node->invtrans = TempMatrix * node->invtrans;
				break;
			case 2:
				caxis = 'y';
				angle = dval;
				TempMatrix.SetRotationMatrix(angle, caxis);
				node->trans = node->trans * TempMatrix;
				TempMatrix.SetRotationMatrix(-angle, caxis);
				node->invtrans = TempMatrix * node->invtrans;
				break;
			case 3:
				caxis = 'z';
				angle = dval;
				TempMatrix.SetRotationMatrix(angle, caxis);
				node->trans = node->trans * TempMatrix;
				TempMatrix.SetRotationMatrix(-angle, caxis);
				node->invtrans = TempMatrix * node->invtrans;
				break;
			case 4:
				TempMatrix.SetScalingMatrix(vec.x, vec.y, vec.z);
				node->trans = node->trans * TempMatrix;
				TempMatrix.SetScalingMatrix(1.0 / vec.x, 1.0 / vec.y, 1.0 / vec.z);
				node->invtrans = TempMatrix * node->invtrans;
				break;
			case 5:
				VisibleNode (node->node_name, dval);
				break;
			default:
				break;
		}
	}
}

const string& CCharShape::GetNodeFullname (size_t idx) const {
	if (idx >= numNodes) return emptyString;
	return Nodes[idx]->action->name;
}

size_t CCharShape::GetNumActs (size_t idx) const {
	if (idx >= numNodes) return -1;
	return Nodes[idx]->action->num;
}

TCharAction *CCharShape::GetAction (size_t idx) const {
	if (idx >= numNodes) return NULL;
	return Nodes[idx]->action;
}

void CCharShape::PrintAction (size_t idx) const {
	if (idx >= numNodes) return;
	TCharAction *act = Nodes[idx]->action;
	PrintInt ((int)act->num);
	for (size_t i=0; i<act->num; i++) {
		PrintInt (act->type[i]);
		PrintDouble (act->dval[i]);
		PrintVector (act->vec[i]);
	}
}

void CCharShape::PrintNode (size_t idx) const {
	TCharNode *node = Nodes[idx];
	PrintInt ("node: ", (int)node->node_name);
	PrintInt ("parent: ", (int)node->parent_name);
	PrintInt ("child: ", (int)node->child_name);
	PrintInt ("next: ", (int)node->next_name);
}

void CCharShape::SaveCharNodes (const string& dir, const string& filename) {
	CSPList list (MAX_CHAR_NODES + 10);

	list.Add ("# Generated by Tuxracer tools");
	list.AddLine ();
	if (!Materials.empty()) {
		list.Add ("# Materials:");
		for (size_t i=0; i<Materials.size(); i++)
			if (!Materials[i].matline.empty())
				list.Add (Materials[i].matline);
		list.AddLine ();
	}

	list.Add ("# Nodes:");
	for (size_t i=1; i<numNodes; i++) {
		TCharNode* node = Nodes[i];
		TCharAction* act = node->action;
		if (node->parent_name >= node->node_name) Message ("wrong parent index");
		string line = "*[node] " + Int_StrN ((int)node->node_name);
		line += " [par] " + Int_StrN ((int)node->parent_name);

		if (!act->order.empty()) {
			bool rotflag = false;
			TVector3d rotation;
			line += " [order] " + act->order;
			for (size_t ii=0; ii<act->order.size(); ii++) {
				int aa = act->order[ii]-48;
				switch (aa) {
					case 0:
						line += " [trans] " + Vector_StrN (act->vec[ii], 2);
						break;
					case 4:
						line += " [scale] " + Vector_StrN (act->vec[ii], 2);
						break;
					case 1:
						rotation.x = act->dval[ii];
						rotflag = true;
						break;
					case 2:
						rotation.y = act->dval[ii];
						rotflag = true;
						break;
					case 3:
						rotation.z = act->dval[ii];
						rotflag = true;
						break;
					case 5:
						line += " [vis] " + Float_StrN (act->dval[ii], 0);
						break;
					case 9:
						rotation.z = act->dval[ii];
						rotflag = true;
						break;
				}
			}
			if (rotflag) line += " [rot] " + Vector_StrN (rotation, 2);
		}
		if (!act->mat.empty()) line += " [mat] " + act->mat;
		if (!node->joint.empty()) line += " [joint] " + node->joint;
		if (!act->name.empty()) line += " [name] " + act->name;
		if (node->render_shadow) line += " [shad] 1";

		list.Add (line);
		if (i<numNodes-3) {
			if (node->visible && !Nodes[i+1]->visible) list.AddLine ();
			const string& joint = Nodes[i+2]->joint;
			if (joint.empty()) list.Add ("# " + joint);
		}
	}
	list.Save (dir, filename);
}
