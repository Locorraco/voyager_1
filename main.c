/*
    nbody
    Copyright (C) 2020 Bruce Hiram Ginori Rodríguez
                       bruginrod@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "element.h"
#include "population.h"
#include "recipes.h"
#include "integrator.h"

int main(int argn, char **args){
  int N = 6;
  Element a1,a2;
  Population population;
  double *distance;
  int i,j,k,m;
  int miproc;
  int numproc;
  int begin,end;
  char filenameIn[256],filenameOut[256];
  FILE *in,*out;
  boolean verbose = true;
  boolean output = false;
  MPI_Status status;
  Element *buffer;
  Element *data;
  Element element;
  MPI_Datatype MPI_ELEMENT;
  double dt;
  MPI_Aint baseaddr,addr1,addr2,addr3;
  MPI_Aint displ[4];
  MPI_Datatype dtype[4];
  int blength[4];
  int response;
  MPI_Op resized_new_type;
  int itera;  
  int steps;
  double Ax,Bx,Ay,By,Az,Bz,radii;
  double au = 1.49598000e8; //km 
  Ax= -10.0*au; //metros
  Bx= 10.0*au;
  Ay= -10.0*au;
  By= 10.0*au;
  Az= -10.0*au;
  Bz= 10.0*au;
  radii=1.0;
  dt=0.2; //sec
  steps=600;


  
  itera = 0;
  
  MPI_Init (&argn, &args); /* Inicializar MPI */
  MPI_Comm_rank(MPI_COMM_WORLD,&miproc); /* Determinar el rango
 del proceso invocado*/
  MPI_Comm_size(MPI_COMM_WORLD,&numproc); /* Determinar el nume
ro de procesos */
  MPI_Barrier (MPI_COMM_WORLD);



  
  // Initial Conditions
  population = new_Population("Voyager-Saturn System",N); // Efemeride 1977 September 06
  // add_element_to_population(&population, new_Element(1.e9, -10.0, -10.0,-10.0, 0.8, 0.8, 0.8));
  add_element_to_population(&population, new_Element(815, 9.679e-1*au, -2.82e-1*au, 1.711e-4*au, 6.683e-3*(au), 2.202e-2*(au), 4.219e-4*(au)));  // Voyager 1 
  add_element_to_population(&population, new_Element(1.989e30, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0));  // Sol
  add_element_to_population(&population, new_Element(5.972e24, 9.669e-1*au, -2.844e-1*au, -6.609e-6*au, 4.582e-3*(au), 1.643e-2*(au), 1.118e-6*(au)));  // Tierra
  add_element_to_population(&population, new_Element(6.4171e23, 8.799e-1*au, 1.19*au, 3.265e-3*au, -1.072e-2*(au), 9.504e-3*(au), 4.63e-4*(au)));  // Marte
  add_element_to_population(&population, new_Element(1.89813e27, 6.999e-1*au, 5.049*au, -3.644e-2*au, -7.570e-3*(au), 1.389e-3*(au), 1.638e-4*(au)));  // Jupiter
  add_element_to_population(&population, new_Element(5.683e26, -7.193*au, 5.707*au, 1.86e-1*au, -3.775e-3*(au), -4.386e-3*(au), 2.266e-4*(au)));  // Saturno


  //Initial Conditions
  if (miproc==0){
    printf("#%i\t%i\t%le\t%le\t%le\t%le\t%le\t%le\t%le\n",steps,population.n_elements,Ax,Bx,Ay,By,Az,Bz,radii);
    printf("#%i\n",itera);
    print_Population(population);    
  }
  
  k=0;
  
  begin = (population.n_elements/numproc)*miproc;
  end = (population.n_elements/numproc)*(miproc+1);

  if (numproc == (miproc+1)){
    if (end < population.n_elements){
      end = population.n_elements;
    }
  }

  data = malloc(sizeof(Element)*(end-begin));

  //buffer is used for the slaves as the list of updated elements
  if (miproc > 0){
    buffer = malloc(sizeof(Element)*(population.n_elements));
  }
  
  MPI_Get_address(&element, &baseaddr);
  MPI_Get_address(&element.mass,&addr1);
  MPI_Get_address(&element.position,&addr2);
  MPI_Get_address(&element.velocity,&addr3);

  displ[0] = 0;
  displ[1] = addr1 - baseaddr;
  displ[2] = addr2 - baseaddr;
  displ[3] = addr3 - baseaddr;

  dtype[0] = MPI_INT; blength[0] = 1;
  dtype[1] = MPI_DOUBLE; blength[1] = 1;
  dtype[2] = MPI_DOUBLE; blength[2] = 3;
  dtype[3] = MPI_DOUBLE; blength[3] = 3;

  MPI_Type_create_struct ( 4, blength, displ, dtype, &MPI_ELEMENT );

////  MPI_Type_create_resized(MPI_ELEMENT,
////                            // lower bound == min(indices) == indices[0]
////                            displ[0],
////                            (MPI_Aint)sizeof(Element),
////                            &resized_new_type);
////    MPI_Type_commit(&resized_new_type);
////    // Free new_type as it is no longer needed
////    MPI_Type_free(&MPI_ELEMENT);


  MPI_Type_commit ( &MPI_ELEMENT );
  
  //m=0;
  //printf("Segmento %i: %i\t%i\n",miproc,begin,end);

  for(itera=1;itera<=steps;itera++){

    /* Note: begin and end are updated for the process 0, we need to be 
       carefoul and in the following cycle of development move the name
       of the variable to improve the computations*/

    if (miproc==0){
      begin = (population.n_elements/numproc)*miproc;
      end = (population.n_elements/numproc)*(miproc+1);
    }
    
    //printf("%i\t%i\t%i\t%i\n",miproc,itera,begin,end);
    response = integrate(population,&data,begin,end,dt);
    //use proc 0
    if (miproc==0){
      for(i=0;i<end;i++){
	      population.element[i] = data[i];
	//print_Element(data[j]);
      }
    }
    /*
      for(i=0;i<population.n_elements-1;i++){
      //for(j=i+1 ;j<population.n_elements;j++){
      for(j=i+1 ;j<population.n_elements;j++){
      if ((begin<=k) && (k<end)){
      //printf("%i,%i\n",i,j);
      population.distance[k] = compute_distance(population.element[i],population.element[j]);
      data[m] = population.distance[k];
      m++;
      }
      k++;
      }
      }
    */
  
    
    
    if (miproc > 0){
      //for(j=0;j<end-begin;j++){ 
      //	printf("before[%i]=%lf\n",j,data[j]);
      //  }
      //data[end-begin-1].position[0] = (double)miproc;
      
      //Sending subset of position to master
      //for(i=begin;i<end; i++){
      //  print_Element(data[i]);
      //}
    
      MPI_Send(data, end-begin, MPI_ELEMENT, 0, 99, MPI_COMM_WORLD);
      //receiving the new list of position of the system from master.
      //MPI_Barrier (MPI_COMM_WORLD);

      MPI_Recv(buffer, population.n_elements , MPI_ELEMENT, 0, 99, MPI_COMM_WORLD, &status);

      for(i=0;i<population.n_elements; i++){
	      population.element[i] = buffer[i];
	//printf("updating info ");
	//print_Element(buffer[i]);
      }
    
    }
    
    else{ // MASTER PROCESS (0)
      for (i = 1; i < numproc; i++){
	      begin = (population.n_elements/numproc)*i;
	      end = (population.n_elements/numproc)*(i+1);
	
	      if (numproc == (i+1)){
	  //printf("last processor %i\t%i\n",end, population.n_elements);
	        if (end < population.n_elements){
	          end = population.n_elements;
	        }
	      }
	      buffer = malloc(sizeof(Element)*(end-begin));
	//Receiving subset of data from slave i
	      MPI_Recv(buffer, end-begin , MPI_ELEMENT, i, 99, MPI_COMM_WORLD, &status);
	
	      k=0;
	      for(j=begin;j<end;j++){ 
	        population.element[j] = buffer[k];
        //printf("receiving elements ");
        //print_Element(population.element[j]);
        //population.distance[begin+j] = buffer[j];
        //printf("distance[%i]=%lf\n",begin+j,population.distance[begin+j]);
        //printf("Recibido [%i] = %lf\n",i,buffer[end-begin-1].position[0]);
	        k++;
	      }
	      free(buffer);
	
      }

      //MPI_Barrier (MPI_COMM_WORLD);

      //Sending updated list of elements to the slaves.
      for (i=1; i < numproc; i++){
	      MPI_Send((&population)->element, population.n_elements, MPI_ELEMENT, i, 99, MPI_COMM_WORLD);
      }
      
      //Print results of the iterations
      printf("#%i\n",itera);
      print_Population(population);    
      
    } //process 0
    
    //  population.distance[0] = compute_distance(population.element[0],population.element[1]);
    
    if(output){
      // write_distances("output/",population,begin,end,miproc);
    }
    
    MPI_Barrier (MPI_COMM_WORLD);
    
    // if (miproc==0){
       //merge all the ouput archives
    
    //// if(output){
    ////   sprintf(filenameOut,"%s/distances.dat","output/");
    ////   out = fopen(filenameOut, "w"); //opening the file for writing.
    ////   for(i=0;i<numproc ;i++){
    ////     sprintf(filenameIn,"%s/distances-%i.dat","output/",i);
    ////     in = fopen(filenameIn, "r"); //opening the file for reading.
    ////     write_block(in,out);
    ////     fclose(in);
    ////   }
    ////   fclose(out);
    ////   }
    ////    if (verbose){
    ////      print_Population(population);
    ////      print_distances(population,0, population.n_distance);
    ////    }
    ////
    ////
    ////    //printf("List of distances %i\n",population.n_distance);
    // }
      
  }//END ITERA
  
  
  MPI_Barrier (MPI_COMM_WORLD);
  //print_distances(population,begin,end);

  if (miproc > 0){
    free(buffer);
  }
  
  MPI_Finalize();
  return 1;
}
