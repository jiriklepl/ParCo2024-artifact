# Noarr Traverser Transformations

This information can also be found in Noarr documentation, but we have compiled a crash-course readme for a quick introduction to transformations used in our experiments.

The traversals performed by Noarr Traversers can be transformed by applying `^ TRANSFORMATION` to the traverser, these transformations include: (the list is non-exhaustive; also, the Noarr library defines many shortcuts for various transformation combinations)

- `into_blocks<OldDim, NewMajorDim, NewMinorDim>(block_size)` - Separates the specified dimension into blocks according to the given block size; each index from the original index space is then uniquely associated with the cartesian product of a major index(block index) and a minor index. The dimension does not have to be contiguous. However, the transformation always assumes the dimension length is divisible by the block size; otherwise, some of the data are associated with indices outside the accessible index space and thus they cannot be traversed.

  For the cases where the original dimension length is not guaranteed to be divisible by the block size, the Noarr library defines:

  - `into_blocks_static<OldDim, NewIsBorderDim, NewMajorDim, NewMinorDim>(block_size)` - The `NewIsBorderDim` is the top-most of the three newly created dimensions, and the other two are dependent on its index. It has a length of `2`; and, if it is given the index `0`, the rest of `into_blocks_static` behaves just like `into_blocks`; if it is, instead, given the index `1`, then the length of the major dimension is set to `1` (there is only one block), and the length of the minor dimension is set to the division remainder (modulo) after dividing the original dimension length by the block size.

  - `into_blocks_dynamic<OldDim, NewMajorDim, NewMinorDim, NewDimIsPresent>(block_size)` - The specified dimension is divided into blocks according to the given block size using round-up division. This means that each index within the original index space is uniquely associated with some major index (block index) and some minor index and it is always accessible within the new index space (contrasting the simple `into_blocks`). Without the *is-present* dimension, this would allow for accessing items outside the original index space - for such items (we can recognize them by `oldLength <= majorIdx * block_size + minorIdx` being `true`), the is-present dimension length is `0`; otherwise, it is `1`. This behavior of the is-present dimension ensures that traversing over the legal items behaves as expected, while the traversal over the items outside the original index space traverses `0` items.

    This essentially emulates a pattern commonly found in algorithms for massively parallelized platforms (such as CUDA), where we map each thread to an element of an array (we assume an array for simplicity - but we can generalize) and then each thread starts its work by checking whether its corresponding index falls within the bounds of the array. For this case, we might use `into_blocks_dynamic<ArrayDim, GridDim, BlockDim, DimIsPresent>` and then use `auto ct = cuda_threads<GridDim, BlockDim>(transformed_traverser)` to bind the appropriate dimensions, the traversal performed by the `ct.inner()` inner traverser then transparently checks the array bound as if we used the `if (x >= array_length) return;` pattern manually.

- `merge_blocks<OldMajor, OldMinor, NewDim>()` - A transformation that performs the transformation inverse to `into_blocks`. It merges the specified major and minor dimensions into a single dimension, effectively applying loop fusion.

- `fix<Dim>(value)`, `fix<Dims...>(values...)`, `fix(state)` - Limits the traversal to a certain row, column, block, etc.

- `bcast<Dim>(length)` - Creates a placeholder dimension that consumes the corresponding input index. The resulting memory access is not affected by the choice of any particular index supplied to the dimension.

  - Typically used as a placeholder when binding dimensions via `cuda_threads`; or when adding blocking to some implementation configurations, we can use `bcast` in other configurations to preserve the same API.

- `rename<OldDim, NewDim>()` - Renames a dimension of a structure. This is especially useful when we want to unify or disconnect dimensions of two or more data structures figuring in a certain traversal. Or, we want to access the same data structure in two different ways.

- `shift<Dim>(offset)` - Makes the index provided for the specified dimension mapped to another one that is shifted by the given offset; also shortens the length of the traversal through the dimension accordingly so all shifted indices match the original range.

- `slice<Dim>(offset, length)` - This is a generalization of `shift` that also explicitly restricts the length of the specified dimension. This is useful whenever we require a traversal through a contiguous subset of indices.

  - `span<Dim>(offset_start, offset_end)` - A similar transformation to `slice` that specifies the end offset instead of the length.

- `hoist<Dim>()` - Moves the specified dimension to the outermost position in the abstracted loop nest. This is equivalent to a loop interchange.

Note that the listed transformations can be applied to memory layouts and traversals alike. The transformations are programmed in such a way they accept and propagate type-embedded literal arguments to enable further optimizations. It is also possible to provide more, user-defined, transformations as needed.
