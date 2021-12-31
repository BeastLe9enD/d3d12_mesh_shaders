struct ASOutput {
    uint Dummy;
};

[NumThreads(1, 1, 1)]
void as_main(uint gtid: SV_GroupThreadID, uint dtid: SV_DispatchThreadID, uint gid: SV_GroupID) {
    ASOutput output;
    output.Dummy = 0;

    DispatchMesh(1, 1, 1, output);
}