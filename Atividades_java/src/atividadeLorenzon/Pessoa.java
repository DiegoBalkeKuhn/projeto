package atividadeLorenzon;


public class Pessoa {
String nome;
int idade;
String cpf;
Endereco endereco;

void imprimirDadosPessoais() {
    System.out.println("Nome: " + nome);
    System.out.println("Idade: " + idade);
    System.out.println("CPF: " + cpf);
    endereco.imprimirEndereco();
}
}
