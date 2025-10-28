Require Import List.
Import ListNotations.

Definition byte := nat.

Definition COM1_BASE : nat := 0x3f8.

Record uart_hw := {
  tx_busy : bool;
}.

Record x86_machine := {
  console : list byte;
  uart : uart_hw
}.

Definition outb (port: nat) (val: byte) (m: x86_machine) :=
  match port with
  | COM1_BASE =>  (* DATA *)
      {| console := m.(console) ++ [val]; uart := m.(uart) |}
  end.

Definition uart_putc (c: byte) (m: x86_machine) :=
  outb 0x3f8 c m.

Definition uart_putc_spec (c: byte) (out: list byte) : list byte :=
  out ++ [c].

Theorem uart_putc_correct:
  forall c m out,
    m.(console) = out ->
    (uart_putc c m).(console) = (uart_putc_spec c out).
Proof.
  intros.
  unfold uart_putc, uart_putc_spec, outb.
  simpl.
  rewrite H.
  reflexivity.
Qed.