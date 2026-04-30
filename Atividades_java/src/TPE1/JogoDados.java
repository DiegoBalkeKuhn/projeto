package TPE1;
public class JogoDados {
    Jogador jogador1;
    Jogador jogador2;
    Dado dado;

    public JogoDados(String nome1, String nome2, int min, int max) {
        jogador1 = new Jogador(nome1);
        jogador2 = new Jogador(nome2);
        dado = new Dado(min, max);
    }

    public void jogar() {
        while (true) {
        int resultado1 = dado.rolar();
        int resultado2 = dado.rolar();
        System.out.println(jogador1.nome + " rolou: " + resultado1);
        System.out.println(jogador2.nome + " rolou: " + resultado2);
        if(checarVitoria(resultado1, resultado2) == "Empate!"){
            System.out.println("Empate! Jogando novamente...");
        } else if(checarVitoria(resultado1, resultado2) == " jogador1 venceu!"){
            System.out.println(jogador1.nome + " venceu!");
            break;
        }
        else{
            System.out.println(jogador2.nome + " venceu!");
            break;
        }
    }
}

    public String checarVitoria(int resultado1, int resultado2) {

        if (resultado1 > resultado2) {
            return " jogador1 venceu!";
        } else if (resultado2 > resultado1) {
            return " jogador2 venceu!";
        } else {
            return "Empate!";
        }
    }
}
