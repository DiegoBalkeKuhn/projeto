package TPE1;

public class Dado {
    int faceMin = 1; // vai mudar de qualquer forma, então meio que foda-se essa inicialização
    int faceMax = 6; // vai mudar de qualquer forma, então meio que foda-se essa inicialização
    int face = 1;

    public Dado(int faceMin, int faceMax) {
        this.faceMin = faceMin;
        this.faceMax = faceMax + 1; // pra incluir o valor máximo
    }

    public int rolar() {
        face = (int)(Math.random() * (faceMax - faceMin)) + faceMin;
        return face;
    }
}
