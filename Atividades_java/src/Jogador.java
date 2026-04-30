public class Jogador {
    String nome;
    int vitorias;
    Dado dado;

    
    public Jogador(String nome, int vitorias) {
        this.nome = nome;
        this.vitorias = vitorias;
        dado = new Dado();
    }

}
