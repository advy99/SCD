#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "Semaphore.h"

using namespace std ;
using namespace SEM ;

const int NUM_FUMADORES = 3;

Semaphore estanquero_puede_trabajar = 1;

//Problema inicializando el vector, de momento, lo tengo que hacer de esta forma.
Semaphore producto_fumadores[NUM_FUMADORES] = {0, 0, 0};


//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
  while ( true ){
    sem_wait ( estanquero_puede_trabajar );

    cout << endl << "El estanquero se pone a trabajar" << endl;

    int producto_generado = aleatorio<0,NUM_FUMADORES-1>();

    cout << endl << "El estanquero pone el producto: " << producto_generado << " y se va a descansar " << endl;

    sem_signal( producto_fumadores[producto_generado] );
  }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
     sem_wait ( producto_fumadores[num_fumador] );

     fumar( num_fumador );

     sem_signal( estanquero_puede_trabajar );

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   thread fumadores[NUM_FUMADORES];
   thread estanquero;

  //Lanzamos hebras
  for (int i = 0; i < NUM_FUMADORES; i++){
    fumadores[i] = thread (funcion_hebra_fumador, i);
  }

  estanquero = thread ( funcion_hebra_estanquero);


  //Esperamos a que acaben las hebras
   for ( int i = 0; i < NUM_FUMADORES; i++){
     fumadores[i].join();
   }

   estanquero.join();
}
