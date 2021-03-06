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

Semaphore puede_producir = 1;

std::vector<Semaphore> ingredientes ;

mutex mtx;


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
   while (true){

		//El estanquero espera a poder producir
      sem_wait(puede_producir); 

		//Cuando puede, produce un elemento
      int producido = aleatorio<0, NUM_FUMADORES-1>();

      mtx.lock();
      cout << endl << "El estanquero produce el item " << producido << endl << flush;
      mtx.unlock();

		//Avisa a los fumadores de que ya hay un ingrediente en la mesa
      sem_signal(ingredientes[producido]);

   }
}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar
	 mtx.lock();
    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;
	 mtx.unlock();

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar
	 mtx.lock();
    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;
	 mtx.unlock();

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
		//Espera a que este su ingrediente
      sem_wait(ingredientes[num_fumador]);

		mtx.lock();
		cout << "El fumador " << num_fumador << " entra a por el ingrediente y lo coge (el estanquero puede trabajar) " << endl;
		mtx.unlock();
		//Cuando esta su ingrediente, avisa de que lo coge y el estanquero
		//puede producir otro

		//Con esto nos aseguramos, que produce un ingrediente cuando ya lo ha retirado un fumador
		sem_signal(puede_producir);

		//El fumador se va a fumar
      fumar(num_fumador);

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   for (int i = 0; i < NUM_FUMADORES; i++)
		ingredientes.push_back(0);

   thread estanquero;

   thread fumadores[NUM_FUMADORES];

   for (int i = 0; i < NUM_FUMADORES; i++)
      fumadores[i] = thread (funcion_hebra_fumador, i);
   
   estanquero = thread (funcion_hebra_estanquero);

   for (int i = 0; i < NUM_FUMADORES; i++)
      fumadores[i].join();

   estanquero.join();
}
