package atividadeLorenzon;

public class Endereco {
    String rua;
    String bairro;
    String cidade;
    String estado;
    String cep;

    void imprimirEndereco() {
        System.out.println(rua + ", " + bairro + ", " + cidade + ", " + estado + ", " + cep);
    }

    public Endereco(String rua, String bairro, String cidade, String estado, String cep) {
        this.rua = rua;
        this.bairro = bairro;
        this.cidade = cidade;
        this.estado = estado;
        this.cep = cep;
    }
}
