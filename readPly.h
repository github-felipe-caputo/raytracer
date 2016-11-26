#ifndef _READPLY_H
#define _READPLY_H

#include <vector>
#include <cstring>

#include "object.h"
#include "libs/ply.h"

typedef struct Vertex {
  float x,y,z;             /* the usual 3-space position of a vertex */
} Vertex;

typedef struct Face {
  unsigned char intensity; /* this user attaches intensity to faces */
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
} Face;

PlyProperty vert_props[] = { /* list of property information for a vertex */
  {(char*) "x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {(char*) "y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {(char*) "z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a vertex */
  //{(char*) "intensity", PLY_UCHAR, PLY_UCHAR, offsetof(Face,intensity), 0, 0, 0, 0},
  {(char*) "vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};

std::vector<Triangle> readPlyFile(std::string filename, Color col) {
    std::vector<Triangle> listOfTriangles;

    // We need a char* for the ply.h lib
    char *charFilename = new char[filename.length()+1];
    std::strcpy(charFilename, filename.c_str());

    PlyFile *ply;

    int nelems;
    char **elist;
    int file_type;
    float version;

    char *elem_name;

    int nprops;
    int num_elems;
    Vertex **vlist;
    Face **flist;

    /* open a PLY file for reading */
    ply = ply_open_for_reading(charFilename, &nelems, &elist, &file_type, &version);

    /* go through each kind of element that we learned is in the file */
    /* and read them */
    for (int i = 0; i < nelems; ++i) {
        /* get the description of the first element */
        elem_name = elist[i];
        ply_get_element_description (ply, elem_name, &num_elems, &nprops);

        /* if we're on vertex elements, read them in */
        if (!std::strcmp("vertex", elem_name)) {
            /* create a vertex list to hold all the vertices */
            vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);

            /* set up for getting vertex elements */
            ply_get_property (ply, elem_name, &vert_props[0]);
            ply_get_property (ply, elem_name, &vert_props[1]);
            ply_get_property (ply, elem_name, &vert_props[2]);

            /* grab all the vertex elements */
            for (int j = 0; j < num_elems; ++j) {

                /* grab and element from the file */
                vlist[j] = (Vertex *) malloc (sizeof (Vertex));
                ply_get_element (ply, (void *) vlist[j]);
            }
        }

        /* if we're on face elements, read them in */
        if (!std::strcmp("face", elem_name)) {
            /* create a list to hold all the face elements */
            flist = (Face **) malloc (sizeof (Face *) * num_elems);

            /* set up for getting face elements */
            ply_get_property (ply, elem_name, &face_props[0]);
            //ply_get_property (ply, elem_name, &face_props[1]);

            /* grab all the face elements */
            for (int j = 0; j < num_elems; ++j) {

                /* grab and element from the file */
                flist[j] = (Face *) malloc (sizeof (Face));
                ply_get_element (ply, (void *) flist[j]);

                // We know it's a triangle
                int a = flist[j]->verts[0];
                int b = flist[j]->verts[1];
                int c = flist[j]->verts[2];

                Point p1(vlist[a]->x, vlist[a]->y, vlist[a]->z);
                Point p2(vlist[b]->x, vlist[b]->y, vlist[b]->z);
                Point p3(vlist[c]->x, vlist[c]->y, vlist[c]->z);

                listOfTriangles.push_back( Triangle(p1,p2,p3,col) );
            }
        }
    }

    /* close the PLY file */
    ply_close (ply);

    return listOfTriangles;
}

#endif
