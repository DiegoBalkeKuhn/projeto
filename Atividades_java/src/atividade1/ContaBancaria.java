package atividade1;

public class ContaBancaria {
  private int numeroConta = 0;
  private double saldoConta = 0.0;

    public ContaBancaria(int numeroConta, double saldoConta) {
        setNumeroConta(numeroConta);
        depositar(saldoConta);
    }

    public int getNumeroConta() {
        return numeroConta;
    }

    public void setNumeroConta(int numeroConta) {
        if(numeroConta <= 0){
            System.out.println("Número da conta deve ser positivo.");
        } else {
        this.numeroConta = numeroConta;
        }
    }

    public double getSaldoConta() {
        return saldoConta;
    }

    public void depositar(double saldoConta) {
        saldoConta += saldoConta;
    }

}
