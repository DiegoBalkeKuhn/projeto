package atividadeLorenzon;
import utilitarios.Teclado2;


public class Aplicacao {
    public static void main(String[] args) {
        
        String nome = Teclado2.read("Nome da empresa: ", String.class);
        String cnpj = Teclado2.read("CNPJ: ", String.class);
        Endereco endereco = new Endereco(
                Teclado2.read("Rua: ", String.class),
                Teclado2.read("Bairro: ", String.class),
                Teclado2.read("Cidade: ", String.class),
                Teclado2.read("Estado: ", String.class),
                Teclado2.read("CEP: ", String.class)
        );        
        Pessoa ceo = new Pessoa();
        Empresa emp = new Empresa(nome, cnpj, endereco, ceo);

        emp.endereco = new Endereco(
                Teclado2.read("Rua: ", String.class),
                Teclado2.read("Bairro: ", String.class),
                Teclado2.read("Cidade: ", String.class),
                Teclado2.read("Estado: ", String.class),
                Teclado2.read("CEP: ", String.class)
        );
        
        System.out.println("Dados do CEO:");
        emp.ceo = new Pessoa();
        emp.ceo.nome = Teclado2.read("Nome do CEO: ", String.class);
        emp.ceo.idade = Teclado2.read("Idade do CEO: ", Integer.class);
        emp.ceo.cpf = Teclado2.read("CPF do CEO: ", String.class);
        
        System.out.println("Endereço do CEO:");
        emp.ceo.endereco = new Endereco(
                Teclado2.read("Rua: ", String.class),
                Teclado2.read("Bairro: ", String.class),
                Teclado2.read("Cidade: ", String.class),
                Teclado2.read("Estado: ", String.class),
                Teclado2.read("CEP: ", String.class)
        );
    }        
}
