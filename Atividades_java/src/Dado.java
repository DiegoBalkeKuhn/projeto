public class Dado {
    public int valor;

    public Dado() {
        this.valor = 1;
    }

    public void rolar() {
        this.valor = (int) (Math.random() * 6) + 1;
    }

    
}
