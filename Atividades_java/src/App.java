import utilitarios.Teclado;

public class App {
    public static void main(String[] args) throws Exception {        
      // criação de variaveis
      int horasTrabalhadas;
      double valorHora;
      //entrada de dados
      System.out.println("Digite as horas trabalhadas:"); //Entrada
      horasTrabalhadas = Teclado.readInt ("");
      System.out.println("Digite o valor da hora trabalhada:"); //Entrada
      valorHora = Teclado.readDouble ("");
      //processamento e saída
      if(horasTrabalhadas <= 160){
        System.out.printf("R$ %.2f\n", (horasTrabalhadas * valorHora) * 0.89); //Saída
      } else {
        System.out.printf("R$ %.2f\n",((horasTrabalhadas * (valorHora * 1.5))) * 0.89); //Saída
      }
      
      System.out.printf("Fim de execução!"); //Saída
    }
}
