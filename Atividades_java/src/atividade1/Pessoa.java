package atividade1;

public class Pessoa {
  private String nome;
  private int idade;
  ContaBancaria conta = new ContaBancaria(0,0);
  
  public Pessoa(String nome, int idade, ContaBancaria conta) {
    setNome (nome);
    setIdade (idade);
    this.conta = conta;
  }

  public String getNome() {
    return nome;
  }

  public void setNome(String nome) {
    this.nome = nome;
  }

  public int getIdade() {
    return idade;
  }

  public void setIdade(int idade) {
    if(idade < 0){
      System.out.println("Idade não pode ser negativa.");
    } else {
    this.idade = idade;
    }
  }
}
