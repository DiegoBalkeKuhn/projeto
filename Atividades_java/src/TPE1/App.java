package TPE1;
import utilitarios.Teclado2;
public class App {
    public static void main(String[] args) {
        String nome1 = Teclado2.read("Digite o nome do jogador 1:", String.class);
        String nome2 = Teclado2.read("digite o nome do jogador 2:", String.class);
        int min = Teclado2.read("Digite o valor mínimo do dado:", Integer.class);
        int max = Teclado2.read("Digite o valor máximo do dado:", Integer.class);

        if (nome1.equals(nome2)) 
        {
            nome1 = nome1 + "1";
            nome2 = nome2 + "2";
        }

        while(true){
            if (min <= 0 || min >= max) {
                min = Teclado2.read("valor minimo invalido, por favor mude-o", Integer.class);
            } else {
                break;
            }
        }

        while(true){
            if (max <= 0 || min >= max) {
                max = Teclado2.read("valor maximo invalido, por favor mude-o", Integer.class);
            } else {
                break;
            }
        }

        JogoDados jogo = new JogoDados(nome1, nome2, min, max);
            jogo.jogar();
    }
}
