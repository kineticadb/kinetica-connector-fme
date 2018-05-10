/*=============================================================================

   Name     : geometryvisitor.cpp

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

#include "geometryvisitor.h"
#include "kineticapriv.h"
#include "kineticawriter.h"
#include "kineticautil.h"

#include <fmetypes.h>
#include <iaggregate.h>
#include <iarc.h>
#include <iarea.h>
#include <iareaiterator.h>
#include <ibox.h>
#include <ibrepsolid.h>
#include <icompositesolid.h>
#include <icompositesurface.h>
#include <icsgsolid.h>
#include <icurveiterator.h>
#include <idonut.h>
#include <iellipse.h>
#include <iextrusion.h>
#include <iface.h>
#include <igeometryiterator.h>
#include <iline.h>
#include <ilogfile.h>
#include <imultiarea.h>
#include <imulticurve.h>
#include <imultipoint.h>
#include <imultisolid.h>
#include <imultisurface.h>
#include <imultitext.h>
#include <ipath.h>
#include <ipoint.h>
#include <ipointiterator.h>
#include <ipolygon.h>
#include <iraster.h>
#include <irectangleface.h>
#include <isegmentiterator.h>
#include <isession.h>
#include <isimpleareaiterator.h>
#include <isimplesoliditerator.h>
#include <isoliditerator.h>
#include <isurfaceiterator.h>
#include <itext.h>
#include <itextiterator.h>
#include <itrianglefan.h>
#include <itrianglestrip.h>

#include <string>

//===========================================================================
// Constructor.
GeometryVisitor::GeometryVisitor(const IFMEGeometryTools* geomTools, IFMESession* session)
:
   fmeGeometryTools_(geomTools),
   fmeSession_(session)
{}

//===========================================================================
// Destructor.
GeometryVisitor::~GeometryVisitor()
{
   //------------------------------------------------------------------------
   // Perform any necessary cleanup
   //------------------------------------------------------------------------
}

//=====================================================================
//
FME_Status GeometryVisitor::visitAggregate(const IFMEAggregate& aggregate)
{
   LOG_KINETICA_INFO( KineticaWriter::gLogFile, kMsgStartVisiting << "aggregate");

   FME_Status badNews;

   // Create iterator to get all geometries
   IFMEGeometryIterator* iterator = aggregate.getIterator();
   while (iterator->next())
   {
      // Create visitor to visit aggregate geometries
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      if (NULL != iterator->getPart())
      {
         badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
         if (badNews)
         {
            // Destroy the iterator
            aggregate.destroyIterator(iterator);
            return FME_FAILURE;
         }
      }
   }
   // Done with the iterator, destroy it
   aggregate.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitPoint(const IFMEPoint& point)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMultiPoint(const IFMEMultiPoint& multipoint)
{
   FME_Status badNews;

   // Create a point iterator
   IFMEPointIterator* iterator = multipoint.getIterator();
   while (iterator->next())
   {
      // Create visitor to visit the next geometry
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         multipoint.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }
   // We are done with the iterator, so destroy it
   multipoint.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitArc(const IFMEArc& arc)
{
   // Determine what kind of arc we are visiting and visit it.
   if (arc.optimalArcTypeRetrieval() == FME_ARC_BY_CENTER_POINT ||
       arc.optimalArcTypeRetrieval() == FME_ARC_BY_CENTER_POINT_START_END)
   {
      return visitArcBCP(arc);
   }
   else if (arc.optimalArcTypeRetrieval() == FME_ARC_BY_BULGE)
   {
      return visitArcBB(arc);
   }
   else if (arc.optimalArcTypeRetrieval() == FME_ARC_BY_3_POINTS)
   {
      return visitArcB3P(arc);
   }

   // If we come to this point we don't know which arc to read, so let's return fail.
   return FME_FAILURE;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitArcBCP(const IFMEArc& arc)
{
   FME_Status badNews;
   IFMEPoint* point = fmeGeometryTools_->createPoint();

   // Create visitor to visit points of the arc
   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   // Get the center point
   badNews = arc.getCenterPoint(*point);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }
   // Visit the center point geometry
   badNews = point->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      // There was an error in visiting point.
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }

   if (arc.hasExplicitEndpoints())
   {
      // Get the start point
      badNews = arc.getStartPoint(*point);
      if (badNews)
      {
         fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
         return FME_FAILURE;
      }
      // Visit the start point geometry
      badNews = point->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
         return FME_FAILURE;
      }

      // Get the end point
      badNews = arc.getEndPoint(*point);
      if (badNews)
      {
         fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
         return FME_FAILURE;
      }
      // Visit the end point geometry
      badNews = point->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
         return FME_FAILURE;
      }
   }

   fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitArcBB(const IFMEArc& arc)
{
   FME_Status badNews;
   IFMEPoint* point = fmeGeometryTools_->createPoint();

   // Create visitor to visit points of the arc
   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   // Get the start point
   badNews = arc.getStartPoint(*point);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }
   // Visit the start point geometry
   badNews = point->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }

   // Get the end point
   badNews = arc.getEndPoint(*point);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }
   // Visit the end point geometry
   badNews = point->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }

   // Get the bulge
   FME_Real64 bulge;
   badNews = arc.getBulge(bulge);
   if (badNews)
   {
      // Bulge is not available because the arc is elliptical or close
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));
      return FME_FAILURE;
   }

   fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(point));

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitArcB3P(const IFMEArc& arc)
{
   LOG_KINETICA_INFO( KineticaWriter::gLogFile, kMsgStartVisiting << "arc by 3 points");

   FME_Status badNews;

   IFMEPoint* startPoint = fmeGeometryTools_->createPoint();
   IFMEPoint* midPoint = fmeGeometryTools_->createPoint();
   IFMEPoint* endPoint = fmeGeometryTools_->createPoint();

   // Create visitor to visit points of the arc
   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   badNews = arc.getPropertiesAs3Points(*startPoint, *midPoint, *endPoint);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(startPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(midPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(endPoint));
      return FME_FAILURE;
   }

   // Visit the start point geometry
   badNews = startPoint->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(startPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(midPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(endPoint));
      return FME_FAILURE;
   }

   // Visit the mid point geometry
   badNews = midPoint->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(startPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(midPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(endPoint));
      return FME_FAILURE;
   }

   // Visit the end point geometry
   badNews = endPoint->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(startPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(midPoint));
      fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(endPoint));
      return FME_FAILURE;
   }

   fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(startPoint));
   fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(endPoint));
   fmeGeometryTools_->destroyGeometry(static_cast<IFMEGeometry*>(midPoint));

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitLine(const IFMELine& line)
{
   LOG_KINETICA_INFO( KineticaWriter::gLogFile, kMsgVisiting << "line" );
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitPath(const IFMEPath& path)
{
   FME_Status badNews;

   // Create iterator to get all segments in the path
   IFMESegmentIterator* iterator = path.getIterator();
   while (iterator->next())
   {
      // There is a segment to the path to add, create a visitor to get the segment string
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy the iterator and exit
         path.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }
   // We are done with the iterator, so destroy it
   path.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMultiCurve(const IFMEMultiCurve& multicurve)
{
   FME_Status badNews;

   // Create an iterator to get the curves
   IFMECurveIterator* iterator = multicurve.getIterator();
   while (iterator->next())
   {
      // Create a visitor to visit the next curve
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy the iterator and return fail
         multicurve.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }
   // Done visiting curves, destroy iterator
   multicurve.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMultiArea(const IFMEMultiArea& multiarea)
{
   FME_Status badNews;

   // Create iterator to visit all areas
   IFMEAreaIterator* iterator = multiarea.getIterator();
   while (iterator->next())
   {
      // Create a visitor to visit areas
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy iterator
         multiarea.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }
   // Done with iterator, destroy it
   multiarea.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitPolygon(const IFMEPolygon& polygon)
{
   FME_Status badNews;

   // Create visitor to visit polygon curve geometry
   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   const IFMECurve* boundary = polygon.getBoundaryAsCurve();
   if (boundary == NULL)
   {
      // We need a boundary, return fail
      return FME_FAILURE;
   }
   badNews = boundary->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitDonut(const IFMEDonut& donut)
{
   FME_Status badNews;

   // Create visitor to visit boundaries
   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   // Get the outer boundary
   const IFMEArea* outerBoundary = donut.getOuterBoundaryAsSimpleArea();
   if (outerBoundary == NULL)
   {
      // We require an outer boundary, return fail
      return FME_FAILURE;
   }
   badNews = outerBoundary->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   // Get the inner boundary
   IFMESimpleAreaIterator* iterator = donut.getIterator();
   while (iterator->next())
   {
      badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         donut.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }
   // Done with iterator, destroy it
   donut.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitText(const IFMEText& text)
{
   FME_Status badNews;

   // Create visitor to visit location
   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   badNews = text.getLocationAsPoint()->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMultiText(const IFMEMultiText& multitext)
{
   FME_Status badNews;

   // Create iterator to get all text geometries
   IFMETextIterator* iterator = multitext.getIterator();
   while (iterator->next())
   {
      // Create visitor to visit next text geometry
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      badNews = iterator->getPart()->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         multitext.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }
   // Done with iterator, destroy it
   multitext.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitEllipse(const IFMEEllipse& ellipse)
{
   FME_Status badNews;

   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   const IFMEArc* boundary = ellipse.getBoundaryAsArc();
   if (boundary == NULL)
   {
      // We require a boundary
      return FME_FAILURE;
   }
   badNews = boundary->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitNull(const IFMENull& fmeNull)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitRaster(const IFMERaster& raster)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitFace(const IFMEFace& face)
{
   FME_Status badNews;

   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   const IFMEArea* area = face.getAsArea();
   if (area == NULL)
   {
      // We require an area
      return FME_FAILURE;
   }

   badNews = area->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitTriangleStrip(const IFMETriangleStrip& triangleStrip)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitTriangleFan(const IFMETriangleFan& triangleFan)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitBox(const IFMEBox& box)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitExtrusion(const IFMEExtrusion& extrusion)
{
   FME_Status badNews;

   LOG_KINETICA_INFO( KineticaWriter::gLogFile, kMsgStartVisiting << "extrusion" );

   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   // Get the base of the extrusion
   const IFMEFace* extrusionBase = extrusion.getBaseAsFace();
   if (extrusionBase == NULL)
   {
      // A base is needed
      return FME_FAILURE;
   }
   badNews = extrusionBase->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitBRepSolid(const IFMEBRepSolid& brepSolid)
{
   FME_Status badNews;

   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   // Get the outer surface
   const IFMESurface* outerSurface = brepSolid.getOuterSurface();
   if (outerSurface == NULL)
   {
      // Need an outer surface
      return FME_FAILURE;
   }
   badNews = outerSurface->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      return FME_FAILURE;
   }

   // Create iterator to loop though all the inner surfaces
   IFMESurfaceIterator* iterator = brepSolid.getIterator();
   while (iterator->next())
   {
      // Get the next inner surface
      const IFMESurface* innerSurface = iterator->getPart();
     
      // Set the inner surface geometry string
      badNews = innerSurface->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy iterator before leaving
         brepSolid.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }

   // Done with the iterator
   brepSolid.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitCompositeSurface(const IFMECompositeSurface& compositeSurface)
{
   FME_Status badNews;

   // Create an iterator to loop through all the surfaces this composite contains
   IFMESurfaceIterator* iterator = compositeSurface.getIterator();
   while (iterator->next())
   {
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      // Get the next surface
      const IFMESurface* surface = iterator->getPart();

      // Set the surface geometry string
      badNews = surface->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy the iterator before leaving
         compositeSurface.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }

   // Done with the iterator
   compositeSurface.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitRectangleFace(const IFMERectangleFace& rectangle)
{
   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMultiSurface(const IFMEMultiSurface& multiSurface)
{
   FME_Status badNews;

   // Create an iterator to loop through all the surfaces this multi surface contains
   IFMESurfaceIterator* iterator = multiSurface.getIterator();
   while (iterator->next())
   {
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      // Get the next surface
      const IFMESurface* surface = iterator->getPart();

      // Set the surface geometry string
      badNews = surface->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy iterator before leaving
         multiSurface.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }

   // Done with the iterator
   multiSurface.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMultiSolid(const IFMEMultiSolid& multiSolid)
{
   FME_Status badNews;

   // Create an iterator to loop through all the solids this multi solid contains
   IFMESolidIterator* iterator = multiSolid.getIterator();
   while (iterator->next())
   {
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      // Get the next solid.
      const IFMESolid* solid = iterator->getPart();

      // Set the solid geometry string
      badNews = solid->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy iterator before leaving
         multiSolid.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }

   // Done with the iterator
   multiSolid.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitCompositeSolid(const IFMECompositeSolid& compositeSolid)
{
   FME_Status badNews;

   LOG_KINETICA_INFO( KineticaWriter::gLogFile, kMsgStartVisiting << "composite solid" );

   // Create an iterator to loop through all the simple solids this composite solid contains
   IFMESimpleSolidIterator* iterator = compositeSolid.getIterator();
   while (iterator->next())
   {
      GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

      // Get next simple solid
      const IFMESimpleSolid* simpleSolid = iterator->getPart();

      // Set the simple solid geometry string
      badNews = simpleSolid->acceptGeometryVisitorConst(visitor);
      if (badNews)
      {
         // Destroy iterator before leaving
         compositeSolid.destroyIterator(iterator);
         return FME_FAILURE;
      }
   }

   // Done with the iterator
   compositeSolid.destroyIterator(iterator);

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitCSGSolid(const IFMECSGSolid& csgSolid)
{
   FME_Status badNews;

   // Convert the IFMECSGSolid to either an IFMEMultiSolid, IFMEBRepSolid, or IFMENull.
   const IFMEGeometry* geomCSGSolid = csgSolid.evaluateCSG();

   GeometryVisitor visitor(fmeGeometryTools_, fmeSession_);

   badNews = geomCSGSolid->acceptGeometryVisitorConst(visitor);
   if (badNews)
   {
      // Destroy the geomCSGSolid before returning
      fmeGeometryTools_->destroyGeometry(const_cast<IFMEGeometry*>(geomCSGSolid));
      return FME_FAILURE;
   }

   // Done with the geomCSGSolid
   fmeGeometryTools_->destroyGeometry(const_cast<IFMEGeometry*>(geomCSGSolid));

   return FME_SUCCESS;
}

//=====================================================================
//
FME_Status GeometryVisitor::visitMesh( const IFMEMesh& mesh )
{
   //TODO: 22012 This method should be written.
   return FME_SUCCESS;
}

//=====================================================================
FME_Status GeometryVisitor::visitPointCloud(const IFMEPointCloud& pointCloud) 
{
   //TODO: PR:26877 This method should be written.
   return FME_SUCCESS;
}

//=====================================================================
FME_Status GeometryVisitor::visitFeatureTable(const IFMEFeatureTable& featureTable)
{
   return FME_SUCCESS;
}
