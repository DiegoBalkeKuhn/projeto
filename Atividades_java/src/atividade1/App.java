package atividade1;

public class App {
  public static void main(String[] args) {
    String nome = "João";
    int idade = 30;
    ContaBancaria conta = new ContaBancaria(12345,10000.0);
    Pessoa pessoa = new Pessoa(nome, idade, conta);

    System.out.println("Nome: " + pessoa.getNome());
    System.out.println("Idade: " + pessoa.getIdade());
    System.out.println("Número da Conta: " + pessoa.conta.getNumeroConta());
    System.out.println("Saldo da Conta: " + pessoa.conta.getSaldoConta());

  }
}
