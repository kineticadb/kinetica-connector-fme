#ifndef GEOMETRY_VISITOR_H
#define GEOMETRY_VISITOR_H
/*=============================================================================

   Name     : geometryvisitor.h

   System   : FME Plug-in SDK

   Language : C++

   Purpose  : Declaration of GeometryVisitor

         Copyright (c) 1994 - 2016, Safe Software Inc. All rights reserved.

   Redistribution and use of this sample code in source and binary forms, with 
   or without modification, are permitted provided that the following 
   conditions are met:
   * Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.

   THIS SAMPLE CODE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SAMPLE CODE, EVEN IF 
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#include <igeometrytools.h>
#include <igeometryvisitor.h>

using namespace std;

class IFMESession;

// This returns a string that contains the geometry of a feature passed in.
class GeometryVisitor : public IFMEGeometryVisitorConst
{
public:

   //---------------------------------------------------------------------
   // Constructor.
   GeometryVisitor(const IFMEGeometryTools* geomTools, IFMESession* session);

   //---------------------------------------------------------------------
   // Destructor.
   ~GeometryVisitor();

   //---------------------------------------------------------------------
   // Return version.
   virtual FME_Int32 getVersion() const
   {
      return kGeometryVisitorVersion; // This constant defined in the parent's header file
   }

   //---------------------------------------------------------------------
   // Visitor logs values of the passed in IFMEAggregate geometry object.
   virtual FME_Status visitAggregate(const IFMEAggregate& aggregate);

   //---------------------------------------------------------------------
   // Visitor logs values of the passed in IFMEPoint geometry object.
   virtual FME_Status visitPoint(const IFMEPoint& point);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEMultiPoint geometry object.
   virtual FME_Status visitMultiPoint(const IFMEMultiPoint& multipoint);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEArc geometry object.
   virtual FME_Status visitArc(const IFMEArc& arc);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMELine geometry object.
   virtual FME_Status visitLine(const IFMELine& line);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEPath geometry object.
   virtual FME_Status visitPath(const IFMEPath& path);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEMultiCurve geometry object.
   virtual FME_Status visitMultiCurve(const IFMEMultiCurve& multicurve);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEMultiArea geometry object.
   virtual FME_Status visitMultiArea(const IFMEMultiArea& multiarea);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEPolygon geometry object.
   virtual FME_Status visitPolygon(const IFMEPolygon& polygon);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEDonut geometry object.
   virtual FME_Status visitDonut(const IFMEDonut& donut);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEText geometry object.
   virtual FME_Status visitText(const IFMEText& text);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEMultiText geometry object.
   virtual FME_Status visitMultiText(const IFMEMultiText& multitext);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEEllipse geometry object.
   virtual FME_Status visitEllipse(const IFMEEllipse& ellipse);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMENull geometry object.
   virtual FME_Status visitNull(const IFMENull& fmeNull);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMERaster geometry object.
   virtual FME_Status visitRaster(const IFMERaster& raster);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEFace geometry object.
   virtual FME_Status visitFace(const IFMEFace& face);

   //---------------------------------------------------------------------
   // Visitor creates a string representing the values of the passed
   // in IFMETriangleStrip geometry object.  It then assigns the string to
   // the "geomString_" data member.
   virtual FME_Status visitTriangleStrip(const IFMETriangleStrip& triangleStrip);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMETriangleFan geometry object.
   virtual FME_Status visitTriangleFan(const IFMETriangleFan& triangleFan);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEBox geometry object.
   virtual FME_Status visitBox(const IFMEBox& box);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEExtrusion geometry object.
   virtual FME_Status visitExtrusion(const IFMEExtrusion& extrusion);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEBRepSolid geometry object.
   virtual FME_Status visitBRepSolid(const IFMEBRepSolid& brepSolid);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMECompositeSurface geometry object.
   virtual FME_Status visitCompositeSurface(const IFMECompositeSurface& compositeSurface);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMERectangleFace geometry object.
   virtual FME_Status visitRectangleFace(const IFMERectangleFace& rectangle);

   //---------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEMultiSurface geometry object.
   virtual FME_Status visitMultiSurface(const IFMEMultiSurface& multiSurface);

   //---------------------------------------------------------------------
   // Visitor clogs the values of the passed in IFMEMultiSolid geometry object.
   virtual FME_Status visitMultiSolid(const IFMEMultiSolid& multiSolid);

   //----------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMECompositeSolid geometry object.
   virtual FME_Status visitCompositeSolid(const IFMECompositeSolid& compositeSolid);

   //----------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMECSGSolid geometry object.
   // The string created does not represent a true IFMECSGSolid, instead it
   // represents the IFMEMultiSolid, IFMEBRepSolid, or IFMENull equivalent to it.
   virtual FME_Status visitCSGSolid(const IFMECSGSolid& csgSolid);

   //----------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEMesh geometry object.
   virtual FME_Status visitMesh(const IFMEMesh& mesh);

   //----------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEPointCloud geometry object.
   virtual FME_Status visitPointCloud(const IFMEPointCloud& pointCloud);

   //----------------------------------------------------------------------
   // Visitor logs the values of the passed in IFMEFeatureTable geometry object.
   virtual FME_Status visitFeatureTable(const IFMEFeatureTable& featureTable);

private:

   //---------------------------------------------------------------
   // Copy constructor
   GeometryVisitor (const GeometryVisitor&);

   //---------------------------------------------------------------
   // Assignment operator
   GeometryVisitor& operator=(const GeometryVisitor&);


   //---------------------------------------------------------------------
   // The IFMEArc geometry object passed in here is an arc by center point.
   // This function logs the values of that IFMEArc.
   FME_Status visitArcBCP(const IFMEArc& arc);

   //---------------------------------------------------------------------
   // The IFMEArc geometry object passed in here is an arc by bulge.
   // This function logs the values of that IFMEArc.
   FME_Status visitArcBB(const IFMEArc& arc);

   //---------------------------------------------------------------------
   // The IFMEArc geometry object passed in here is an arc by 3 points.
   // This function logs the values of that IFMEArc.
   FME_Status visitArcB3P(const IFMEArc& arc);


   // The fmeGeometryTools member stores a pointer to an IFMEGeometryTools
   // object that is used to create IFMEGeometries.
   const IFMEGeometryTools* fmeGeometryTools_;

   // The fmeSession_ member stores a pointer to an IFMESession object
   // which performs the services on the FME Objects.
   IFMESession* fmeSession_;

};

#endif
