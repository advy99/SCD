#include <iostream>
#include <cassert>
#include <thread>
//#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo


#include "HoareMonitor.h"


using namespace std ;
using namespace HM;

const int NUM_FUMADORES = 3;

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


int producirIngrediente(){
   return aleatorio<0,2>();
}



class Estanco : public HoareMonitor{
private:
   int ingrediente_en_mostrador;
   CondVar estanquero;
   CondVar fumadores[NUM_FUMADORES];

public:
   Estanco();
   void ponerIngrediente(int ingrediente);
   void esperarRecogidaIngrediente();
   int obtenerIngrediente(int n_fumador);
};

Estanco::Estanco(){
   ingrediente_en_mostrador = NUM_FUMADORES + 1;
   estanquero = newCondVar();
   for (int i = 0; i < NUM_FUMADORES; i++){
      fumadores[i] = newCondVar();
   }
}

void Estanco::ponerIngrediente(int ingrediente){

   if (ingrediente_en_mostrador != NUM_FUMADORES + 1){
      estanquero.wait();
   }
   mtx.lock();
   cout << "El estanquero pone el ingrediente " << ingrediente << endl << flush;
   mtx.unlock();
   ingrediente_en_mostrador = ingrediente;

   fumadores[ingrediente].signal();

}

void Estanco::esperarRecogidaIngrediente(){


   if (ingrediente_en_mostrador != NUM_FUMADORES + 1){
      estanquero.wait();
   }



}

int Estanco::obtenerIngrediente(int n_fumador){
   int ingrediente;
   while (ingrediente_en_mostrador != n_fumador){
      fumadores[n_fumador].wait();
   }

   ingrediente = ingrediente_en_mostrador;


   ingrediente_en_mostrador = NUM_FUMADORES + 1;

   estanquero.signal();

   return ingrediente;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero( MRef<Estanco> estanco )
{


   int ingrediente;
   while (true){
      ingrediente = producirIngrediente();
      estanco->ponerIngrediente(ingrediente);
      estanco->esperarRecogidaIngrediente();

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
void  funcion_hebra_fumador( MRef<Estanco> estanco, int num_fumador )
{
   while( true )
   {

      while (true){
         estanco->obtenerIngrediente(num_fumador);
         fumar(num_fumador);
      }

   }
}

//----------------------------------------------------------------------

int main()
{
   // declarar hebras y ponerlas en marcha
   // ......

   MRef<Estanco> estanco = Create<Estanco> ();

   thread estanquero;

   thread fumadores[NUM_FUMADORES];

   for (int i = 0; i < NUM_FUMADORES; i++)
      fumadores[i] = thread (funcion_hebra_fumador, estanco, i);

   estanquero = thread (funcion_hebra_estanquero, estanco);

   for (int i = 0; i < NUM_FUMADORES; i++)
      fumadores[i].join();

   estanquero.join();
}
