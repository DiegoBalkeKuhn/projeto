import utilitarios.Teclado2;

public class jogo {
    public static void main(String[] args) {
        String nome1 = Teclado2.read("Digite o nome do primeiro jogador:", String.class);
        String nome2 = Teclado2.read("Digite o nome do segundo jogador:", String.class);
        int vitorias1 = 0;
        int vitorias2 = 0;

        Jogador jogador1 = new Jogador(nome1, vitorias1);
        Jogador jogador2 = new Jogador(nome2, vitorias2);

        while(true) {
        jogador1.dado.rolar();
        jogador2.dado.rolar();

        if (jogador1.dado.valor > jogador2.dado.valor) {
            System.out.println(jogador1.nome + " venceu com o dado \n" + jogador1.dado.valor);
            jogador1.vitorias++;
            break;
        } else if (jogador2.dado.valor > jogador1.dado.valor) {
            System.out.println(jogador2.nome + " venceu com o dado \n" + jogador2.dado.valor);
            jogador2.vitorias++;
            break;
        } else {
            System.out.println("Empate! Ambos os jogadores rolaram \n " + jogador1.dado.valor);
            continue;
        }
    }

    }
}
