package MAV-2.0;

import exemplos.Exemplo1;
import utilitarios.Teclado;

public class App {
    public static void main(String[] args) throws Exception {        

        Exemplo1 exemplo;
        int valor;

        valor = Teclado.readInt("Informe um valor:");
        exemplo = new Exemplo1(valor);    

        System.out.printf ("%s\n", exemplo); //toString
        System.out.println("Fim de execução!"); //Saída
    }
}
