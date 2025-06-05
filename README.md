# Interleaving Static Analysis and LLM Prompting 

The contents of this repository are broken into three directories:
- [The tool implementation](./ErrorSpecifications/)
- [Papers](./paper/)
- [Results from paper](./results/)

### Interleaved Analysis Tool

To re-run the experiments that were presented in the paper or to run your own,
view the [README](./ErrorSpecifications/README.md) in the [ErrorSpecifications](./ErrorSpecifications/)
directory. Please note that running the interleaved analysis with the LLM is
inherently non-deterministic, so the per-run results will likely be different.

### Papers

The paper `Interleaving Static Analysis and LLM Prompting` that was published
at *SOAP 2024* is included at [paper/soap24_paper.pdf](paper/soap24_paper.pdf).
The paper `Interleaving Static Analayis and LLM Prompting with Applications
to Error Specification Inference` from *STTT 2025* is included at [paper/sttt25_paper.pdf](paper/sttt25_paper.pdf).

### Results from Presented Experimental Results

We have included the raw experimental results for the inferred error specifications
in the [results](./results/) directory. As the interleaved analysis is
non-deterministic, we include these results separately as re-running the analysis
may yield different results.
